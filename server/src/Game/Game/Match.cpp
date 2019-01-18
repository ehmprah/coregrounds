#include "stdafx.h"
#include "Match.hpp"
#include "Data/Game.hpp"
#include "GameStatistics.hpp"
#include "Game/Phase/GamePhaseNormal.hpp"
#include "Core/AI.hpp"

#include "Game/controller/AIController.hpp"
#include "Game/controller/PlayerController.hpp"
#include "Game/controller/Handler.hpp"
#include <AI/AICreator.hpp>

namespace game
{
	phase::GamePtr _createGameState(MatchDef::GameMode _mode, const std::vector<Team>& _teams, const GameData& _gameData)
	{
		return std::make_unique<phase::GameNormal>(_teams, _gameData);
	}

	phase::PickPtr _createPickState(MatchDef::PickMode _mode, const std::vector<Team>& _teams, const GameData& _gameData)
	{
		return std::make_unique<phase::Pick>(_teams, _gameData);
	}

	controller::ControllerPtr _createController(const MatchDef::ParticipantDef& _definition, int _index, controller::InputHandler& _inputHandler,
		const GameData& _gameData, phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase)
	{
		struct Visitor
		{
			int index;
			const GameData& gameData;
			controller::InputHandler& inputHandler;
			phase::Pick& pickPhase;
			phase::AbstractGame& gamePhase;
			phase::Finalize& finalizePhase;

			controller::ControllerPtr operator()(int _botLevel)
			{
				ai::Initializer initializer{ *gameData, _botLevel };
				return std::make_unique<controller::AI>(index, game::ai::Creator::loadAI(initializer), inputHandler, pickPhase, gamePhase, finalizePhase);
			}

			controller::ControllerPtr operator()(std::weak_ptr<game::controller::Handler> _handler)
			{
				if (auto ptr = _handler.lock())
				{
					auto controller = std::make_unique<controller::Player>(index, _handler, inputHandler, pickPhase, gamePhase, finalizePhase);
					ptr->setController(*controller);
					return controller;
				}
				else
					LOG_ERR("Match::createController: unable to create PlayerController: handler is invalid");
				return nullptr;
			}
		};
		return std::visit(Visitor{ _index, _gameData, _inputHandler, _pickPhase, _gamePhase, _finalizePhase }, _definition);
	}

	std::vector<controller::ControllerPtr> _createControllers(const MatchDef& _definition, controller::InputHandler& _inputHandler,
		const GameData& _gameData, phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase)
	{
		std::vector<controller::ControllerPtr> controller;
		for (std::size_t i = 0; i < _definition.participants.size(); ++i)
		{
			controller.emplace_back(_createController(_definition.participants[i], i, _inputHandler, _gameData, _pickPhase, _gamePhase, _finalizePhase));
		}
		return controller;
	}

	MatchPtr createMatch(MatchDef _matchDef, const GameData& _gameData)
	{
		return std::make_unique<Match>(std::move(_matchDef), _gameData);
	}

	std::vector<Team> _createTeams(MatchDef _matchDef)
	{
		std::vector<Team> teams;
		std::transform(std::begin(_matchDef.participants), std::end(_matchDef.participants), std::back_inserter(teams),
			[index = 0](auto& _participant) mutable
			{
				Team team(index + 1);
				team.addMember(index++);
				return team;
			}
		);
		return teams;
	}

	Match::Match(MatchDef _matchDef, const GameData& _gameData) :
		m_MatchDef(std::move(_matchDef)),
		m_Teams(_createTeams(m_MatchDef)),
		m_PickPhase(_createPickState(m_MatchDef.pickMode, m_Teams, _gameData)),
		m_GamePhase(_createGameState(m_MatchDef.gameMode, m_Teams, _gameData)),
		m_FinalizePhase(std::make_unique<phase::Finalize>(m_Teams, *_gameData)),
		m_InputHandler(*m_PickPhase, *m_GamePhase, *m_FinalizePhase)
	{
		assert(0 < m_MatchDef.tempId);

		assert(m_PickPhase && m_GamePhase && m_FinalizePhase);
		m_Controllers = _createControllers(m_MatchDef, m_InputHandler, _gameData, *m_PickPhase, *m_GamePhase, *m_FinalizePhase);

		assert(m_Controllers.size() == 2 && std::none_of(std::begin(m_Controllers), std::end(m_Controllers),
			[](const auto& _controller) { return _controller == nullptr; }
		));
		_switchNextPhase();
	}

	Statistics Match::_generateStatistics() const
	{
		assert(m_GamePhase->hasFinished());
		Statistics result;
		result.tempMatchId = m_MatchDef.tempId;
		result.duration = std::chrono::duration_cast<std::chrono::seconds>(m_GamePhase->getGameTimeElapsed());
		result.winnerTeam = m_GamePhase->getWinningTeamId();
		result.matchType = m_MatchDef.readableMatchType;

		auto& participants = m_GamePhase->getParticipants();
		result.players.reserve(std::size(participants));
		std::transform(std::begin(participants), std::end(participants), std::back_inserter(result.players),
			[](auto& _participant) { return _participant->gatherStatistics(); }
		);
		return result;
	}

	std::vector<FinalizeParticipantPtr> _generateFinalizeParticipants(const phase::AbstractGame::Participants& _participants, phase::Finalize& _phase)
	{
		std::vector<FinalizeParticipantPtr> result(std::size(_participants));
		for (std::size_t i = 0; i < std::size(_participants); ++i)
			result[i] = std::make_unique<FinalizeParticipant>(static_cast<int>(i), _participants[i]->getDefinition(), _phase);
		return result;
	}

	void Match::_switchNextPhase()
	{
		if (m_CurrentPhase)
			m_CurrentPhase->finalize();
		switch (m_State)
		{
		case StateType::notStarted:
		{
			std::vector<const ParticipantData*> data;
			data.reserve(m_Controllers.size());
			std::transform(std::begin(m_Controllers), std::end(m_Controllers), std::back_inserter(data), [](const auto& _controller) {
				return &_controller->getParticipantData();
			});

			m_PickPhase->setup(std::move(data));
			m_CurrentPhase = m_PickPhase.get();
			m_State = StateType::pick;
			break;
		}
		case StateType::pick:
		{
			m_GamePhase->setup(m_PickPhase->generateParticipantDefinitions());
			m_CurrentPhase = m_GamePhase.get();
			m_State = StateType::game;
			break;
		}
		case StateType::game:
			m_FinalizePhase->setup(_generateStatistics(), _generateFinalizeParticipants(m_GamePhase->getParticipants(), *m_FinalizePhase));
			m_CurrentPhase = m_FinalizePhase.get();
			m_State = StateType::finalize;
			break;
		case StateType::finalize:
			m_CurrentPhase = nullptr;
			m_State = StateType::finished;
			break;
		case StateType::finished:
			break;
		default:
			throw std::runtime_error("invalid phase.");
		}
		if (m_CurrentPhase)
		{
			m_CurrentPhase->start();
			for (auto& controller : m_Controllers)
				controller->setPhase(m_CurrentPhase->getType());
		}
	}

	Match::State Match::update(Time _diff)
	{
		assert(isValid());

		if (m_CurrentPhase)
		{
			m_InputHandler.exec(m_CurrentPhase->getType());
			auto stateData = m_CurrentPhase->update(_diff);
			for (auto& controller : m_Controllers)
				controller->update(_diff);

			if (m_CurrentPhase->hasFinished())
				_switchNextPhase();

			return { m_State, std::move(stateData) };
		}
		return { m_State };
	}

	bool Match::isValid() const
	{
		return m_PickPhase && m_GamePhase && m_FinalizePhase;
	}
} // namespace game
