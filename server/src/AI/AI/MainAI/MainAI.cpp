#include "stdafx.h"
#include "MainAI.hpp"

namespace game::ai 
{

	const data::AiMazeLayout& chooseLayout(const data::AiStrategy& m_Strategy, const data::Game& _gameData)
	{
		// Choose one layout at random
		auto& layoutId = getRandomElement(m_Strategy.mazeLayouts);
		// Get it from the game data, assert and return
		auto layout = data::findData(_gameData.aiMazeLayouts, layoutId);
		assert(layout != nullptr);
		return *layout;
	}

	const data::AiStrategy& chooseStrategy(const data::Game& _gameData, int _botLevel)
	{
		// Get all strategies
		auto& strategies = _gameData.aiStrategies;
		assert(!strategies.empty());
		// Get all strategies which match the current botlevel
		std::vector<const data::AiStrategy*> strategiesInRange;
		for (auto& strategy : strategies)
		{
			assert(strategy.botLevelMin < strategy.botLevelMax);
			if (_botLevel >= strategy.botLevelMin && _botLevel <= strategy.botLevelMax)
			{
				strategiesInRange.emplace_back(&strategy);
			}
		}
		// And of those, choose one at random
		return *getRandomElement(strategiesInRange);
	}

	std::unique_ptr<WallStrategy> createWallStrategy(const data::AiMazeLayout* _layout)
	{
		if (_layout != nullptr) return std::make_unique<LayoutWalls>(*_layout);
		return std::make_unique<RandomWalls>();
	}

	int getReactionTime(int _botLevel) 
	{
		// We start with 100ms
		int reactionTime = 100;
		// We add up to four seconds based on bot level
		reactionTime += std::lround((4000 * ((100 - _botLevel) / 100.0)));
		// And we add another half second for each level below 6
		reactionTime += std::max<int>(0, (6 - _botLevel) * 500);
		return reactionTime;
	}

	int getNextPick(const std::vector<IDs>& _idList, const google::protobuf::RepeatedField<google::protobuf::uint32>& _availablePicks, int _currentIndex)
	{
		assert(0 <= _currentIndex && !std::empty(_availablePicks));
		// We start at the current index, but we loop over remaining picks in the set before we resort to random
		for (; _currentIndex < std::size(_idList); ++_currentIndex) 
		{
			for (auto id : _idList[_currentIndex])
			{
				if (std::find(std::begin(_availablePicks), std::end(_availablePicks), id) != std::end(_availablePicks))
					return id;
			}
		}
		return getRandomElement(_availablePicks);
	}

	MainAI::MainAI(const Initializer& _initializer) :
		BaseAI(_initializer),
		m_Strategy(chooseStrategy(_initializer.gameData, _initializer.botLevel)),
		m_Layout(chooseLayout(m_Strategy, _initializer.gameData)),
		m_ParticipantData(generateParticipantData(_initializer.gameData, _initializer.botLevel, &m_Strategy)),
		m_BotLevel(_initializer.botLevel),
		m_ReactionOffset(getReactionTime(m_BotLevel)),
		m_StrategyFlags(translateFlags(m_Strategy.flags)),
		m_WallStrategy(createWallStrategy(&m_Layout)),
		m_MinionStrategy(std::make_unique<MinionDefault>(m_BotLevel, m_StrategyFlags, m_HeatMap, *_initializer.gameData.gameProperties.game.minionLimit)),
		m_TowerStrategy(std::make_unique<TowerDefault>(m_BotLevel, m_StrategyFlags, m_HeatMap)),
		m_AbilityStrategy(std::make_unique<AbilityDefault>(m_BotLevel, m_StrategyFlags, m_HeatMap))
	{
		LOG_INFO("Chosen strategy " + m_Strategy.name.getValue());
		m_ReactionTimer.start(Time(500));
	}

	const ParticipantData& MainAI::getParticipantData() const
	{
		return m_ParticipantData;
	}

	std::optional<protobuf::in::Match_Game> MainAI::update(Time _diff, const protobuf::out::Match_Game& _msg) 
	{
		// A nice bot says glhf
		if (!m_EmoteActivations.glhf && 0 < _msg.gametime())
		{
			protobuf::in::Match_Game inputMsg;
			auto& activation = *inputMsg.add_emoteplays();
			activation.set_id(EmoteIds::GoodLuckHaveFun);
			m_EmoteActivations.glhf = true;
			return std::move(inputMsg);
		}

		// We only act if the necessary reaction time has passed			
		if (m_ReactionTimer.isActive()) 
		{
			m_ReactionTimer.update(_diff);
			return std::nullopt;
		}

		// If we've made it past the reaction barrier, restart the timer and continue the loop
		m_ReactionTimer.start(Time(m_ReactionOffset));

		// We invalidate previously generated heatmaps, but cache them for the remainder of the tick
		m_HeatMap.update(_msg);

		// All strategies loop over all available factories, almost all return after the first input
		protobuf::in::Match_Game inputMsg;
		bool hasInput = false;
		auto res = m_MinionStrategy->update(inputMsg, _msg);

		if (res.shouldReturn) return std::move(inputMsg);
		if (res.hasInput) hasInput = true;

		res = m_TowerStrategy->update(inputMsg, _msg);
		if (res.shouldReturn) return std::move(inputMsg);
		if (res.hasInput) hasInput = true;

		res = m_AbilityStrategy->update(inputMsg, _msg);
		if (res.shouldReturn) return std::move(inputMsg);
		if (res.hasInput) hasInput = true;

		res = m_WallStrategy->update(inputMsg, _msg);
		if (res.shouldReturn) return std::move(inputMsg);
		if (res.hasInput) hasInput = true;

		// TODO set global target (core, support units, towers)?

		if (hasInput) return std::move(inputMsg);
		return std::nullopt;
	}

	std::optional<protobuf::in::Match_Pick> MainAI::update(Time _diff, const protobuf::out::Match_Pick& _msg)
	{
		// A nice bot says hello
		if (!m_EmoteActivations.hello) 
		{
			protobuf::in::Match_Pick inputMsg;
			auto& activation = *inputMsg.add_emoteplays();
			activation.set_id(EmoteIds::Hello);
			m_EmoteActivations.hello = true;
			return std::move(inputMsg);
		}

		// We only act if the necessary reaction time has passed			
		if (m_ReactionTimer.isActive()) 
		{
			m_ReactionTimer.update(_diff);
			return std::nullopt;
		}

		// During the pick phase, we wait around a second before picking
		m_ReactionTimer.start(Time(randomInt(500, 1500)));

		bool hasInput = false;
		protobuf::in::Match_Pick inputMsg;

		int state = _msg.state();

		// Set modification page
		auto playerIndex = removeDirtyFlag(_msg.playerindex());
		auto& player = _msg.player(playerIndex);
		if (_msg.state() == protobuf::out::Match_Pick_State::Match_Pick_State_modSelection)
		{
			auto id = player.modpageid();
			const int activePageId = 2;
			// We use modifications starting from botlevel 50
			if (50 <= m_BotLevel && id != activePageId)
			{
				auto& modpage = *inputMsg.add_modpageselections();
				modpage.set_pid(activePageId);
				hasInput = true;
			}
		}

		// If it's not our turn, we don't do shit
		if (playerIndex != removeDirtyFlag(_msg.activeplayerindex()))
			return std::nullopt;

		auto& availablePicks = player.availableids();
		switch (_msg.state())
		{
			case protobuf::out::Match_Pick_State::Match_Pick_State_factoryBan:
			{
				auto& factoryBan = *inputMsg.add_bans();
				factoryBan.set_target(protobuf::in::Match_Pick_Target_factory);
				if (30 < m_BotLevel) 
				{
					factoryBan.set_id(getNextPick(m_Strategy.factoryBans, availablePicks, 0));
				}
				else 
				{
					factoryBan.set_id(getRandomElement(availablePicks));
				}
				hasInput = true;
				break;
			}
			case protobuf::out::Match_Pick_State::Match_Pick_State_abilityBan:
			{
				auto& abilityBan = *inputMsg.add_bans();
				abilityBan.set_target(protobuf::in::Match_Pick_Target_ability);
				if (30 < m_BotLevel)
				{
					abilityBan.set_id(getNextPick(m_Strategy.abilityBans, availablePicks, 0));
				}
				else
				{
					abilityBan.set_id(getRandomElement(availablePicks));
				}
				hasInput = true;
				break;
			}
			case protobuf::out::Match_Pick_State::Match_Pick_State_factoryPick:
			{
				auto curIndex = _msg.player(playerIndex).factorypicks_size();
				auto& factoryPick = *inputMsg.add_picks();
				factoryPick.set_target(protobuf::in::Match_Pick_Target_factory);
				factoryPick.set_id(getNextPick(m_Strategy.factoryPicks, availablePicks, curIndex));
				hasInput = true;
				break;
			}
			case protobuf::out::Match_Pick_State::Match_Pick_State_abilityPick:
			{
				auto curIndex = _msg.player(playerIndex).abilitypicks_size();
				auto& factoryPick = *inputMsg.add_picks();
				factoryPick.set_target(protobuf::in::Match_Pick_Target_ability);
				factoryPick.set_id(getNextPick(m_Strategy.abilityPicks, availablePicks, curIndex));
				hasInput = true;
				break;
			}
		}

		if (hasInput)
			return std::move(inputMsg);

		return std::nullopt;
	}

	std::optional<protobuf::in::Match_Finalize> MainAI::update(Time _diff, const protobuf::out::Match_Finalize& _msg)
	{
		if (!m_EmoteActivations.gg)
		{
			protobuf::in::Match_Finalize inputMsg;
			auto& activation = *inputMsg.add_emoteplays();
			activation.set_id(EmoteIds::GoodGame);
			m_EmoteActivations.gg = true;
			return std::move(inputMsg);
		}
		return std::nullopt;
	}
} 

