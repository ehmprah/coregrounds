#include "stdafx.h"
#include "AbstractGamePhase.hpp"
#include "Game/Entity/Core.hpp"
#include "Data/Game.hpp"
#include "GameStatistics.hpp"

namespace game::phase
{
	AbstractGame::AbstractGame(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _data) :
		super(),
		m_GameData(_data),
		m_Teams(_teams),
		m_GameProperties(_data->gameProperties.game),
		m_GameDependencies(*this, m_World),
		m_ColliderMap(*_data->gameProperties.game.fieldSize),
		m_TileMap(*_data->gameProperties.game.fieldSize),
		m_PathMap(m_TileMap),
		m_WarmUpTimer(*_data->gameProperties.game.warumUpTime)
	{
	}

	Time AbstractGame::getGameTimeElapsed() const
	{
		if (m_WarmUpTimer.isActive())
			return Time(0);
		if (m_GameBegin <= m_GameEnded)
			return m_GameEnded - m_GameBegin;
		return std::chrono::time_point_cast<Time>(std::chrono::steady_clock::now()) - m_GameBegin;
	}

	std::optional<int> AbstractGame::getWinningTeamId() const
	{
		int notLostCounter = 0;
		int id = 0;
		for (auto& team : m_Teams)
		{
			if (!isTeamDefeated(team, m_Participants))
			{
				++notLostCounter;
				id = team.getId();
			}
		}
		switch (notLostCounter)
		{
		case 0:		// draw happened
			return 0;
		case 1:
			return id;
		}
		return std::nullopt;
	}

	spell::SpellManager& AbstractGame::getSpellManager()
	{
		return m_SpellManager;
	}

	const data::Game& AbstractGame::getGameData() const
	{
		return *m_GameData;
	}

	map::ColliderMap& AbstractGame::getColliderMap()
	{
		return m_ColliderMap;
	}

	map::TileMap& AbstractGame::getTileMap()
	{
		return m_TileMap;
	}

	map::PathMap& AbstractGame::getPathMap()
	{
		return m_PathMap;
	}

	unit::World& AbstractGame::getWorld()
	{
		return m_World;
	}

	const unit::World& AbstractGame::getWorld() const
	{
		return m_World;
	}

	const AbstractGame::Participants& AbstractGame::getParticipants() const
	{
		return m_Participants;
	}

	void AbstractGame::setup(std::vector<ParticipantDef> _definitions)
	{
		assert(_definitions.size() == m_Participants.size());
		for (std::size_t i = 0; i < _definitions.size(); ++i)
		{
			auto& def = _definitions[i];
			def.team = findTeam(m_Teams, i);
			assert(def.team);
			switch (i)
			{
			case 0:
				def.corePosition = TilePosition(0, m_GameProperties.fieldSize->getY() - 1);
				break;
			case 1:
				def.corePosition = TilePosition(m_GameProperties.fieldSize->getX() - 1, 0);
				break;
			}
			m_Participants[i] = std::make_unique<Participant>(static_cast<int>(i), ParticipantInitializer{ def, m_GameDependencies });
		}
		m_PathMap.setCorePositions({ _definitions[0].corePosition, _definitions[1].corePosition });

		m_VisualCollector.resize(m_Participants.size());
	}

	void AbstractGame::start()
	{
		assert(isValid());
		for (auto& participant : m_Participants)
			participant->start();

		// ToDo: init map
	}

	void AbstractGame::finalize()
	{
		assert(isValid());

		LOG_DEBUG("GameState finished.");
	}

	Data AbstractGame::update(Time _diff)
	{
		assert(isValid());

		if (!hasFinished())
		{
			if (m_WarmUpTimer.isActive())
			{
				m_WarmUpTimer.update(_diff);
				m_World.update(_diff, true);
				if (!m_WarmUpTimer.isActive())
				{
					m_GameBegin = std::chrono::time_point_cast<Time>(std::chrono::steady_clock::now());
					LOG_DEBUG("GameState started.");
				}
			}
			else
			{
				m_SpellManager.update(_diff);
				m_World.update(_diff, false);
			}
			for (auto& participant : m_Participants)
				participant->update(_diff);

			m_VisualCollector.swap();

			if (auto id = getWinningTeamId())
			{
				m_Finished = true;
				m_GameEnded = std::chrono::time_point_cast<Time>(std::chrono::steady_clock::now());
			}
		}
		return GameState();
	}

	bool AbstractGame::hasFinished() const
	{
		return m_Finished;
	}

	bool AbstractGame::isValid() const
	{
		return m_GameData && m_Participants[0] && m_Participants[1];
	}

	VisualCollector& AbstractGame::getVisualCollector()
	{
		return m_VisualCollector;
	}

	const game::VisualCollector& AbstractGame::getVisualCollector() const
	{
		return m_VisualCollector;
	}

	Type AbstractGame::getType() const
	{
		return Type::game;
	}

	const Timer& AbstractGame::getWarmUpTimer() const
	{
		return m_WarmUpTimer;
	}

	bool isRunning(const AbstractGame& _phase)
	{
		return !_phase.hasFinished() && !_phase.getWarmUpTimer().isActive();
	}
} // namespace game::phase
