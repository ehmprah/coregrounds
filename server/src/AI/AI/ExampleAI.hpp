#pragma once

#include "BaseAI.hpp"
#include "Utility.hpp"
#include "Core/ParticipantData.hpp"

namespace game::ai
{
	class ExampleAI :
		public BaseAI
	{
	public:
		ExampleAI(const Initializer& _initializer) :
			BaseAI(_initializer),
			m_ParticipantData(generateParticipantData(_initializer.gameData, 0, {}))
		{
			m_WallBuildTimer.start(Time(randomInt<int>(m_MinReactionTime, m_MaxReactionTime)));
		}

		const ParticipantData& getParticipantData() const override
		{
			return m_ParticipantData;
		}

		std::optional<protobuf::in::Match_Game> update(Time _diff, const protobuf::out::Match_Game& _msg) override
		{
			bool hasInput = false;
			protobuf::in::Match_Game inputMsg;

			auto playerIndex = removeDirtyFlag(_msg.playerindex());
			assert(0 <= playerIndex && playerIndex < static_cast<std::uint32_t>(_msg.player_size()));
			auto& player = _msg.player(playerIndex);

			int index = 0;
			for (auto& factory : player.factories())
			{
				if (factory.has_wall() && factory.wall().stacks() > 0)
				{
					if (auto pos = _buildWall(_diff, _msg))
					{
						auto& activation = *inputMsg.add_factoryactivations();
						activation.set_index(index);
						activation.set_position(serializeTilePos(*pos));
						hasInput = true;
					}
				}
				else if (factory.has_minion() && factory.minion().stacks() > 0)
				{
					// we don't have to specify a position here; minions will always spawned at core position.
					auto& activation = *inputMsg.add_factoryactivations();
					activation.set_index(index);
					hasInput = true;
				}
				// check if has no tower built
				else if (factory.has_tower() && factory.tower().guid() == 0 &&
					!player.walls().empty())
				{
					auto& wall = player.walls(randomInt(0, player.walls_size() - 1));
					auto& activation = *inputMsg.add_factoryactivations();
					activation.set_index(index);
					activation.set_position(serializeTilePos(deserializeTilePos(wall.unit().position())));	// this is an important step! the unit positions contains a dirty flag, the activation pos NOT!
																										// we have to eliminate this flag.
					hasInput = true;
				}
				++index;
			}

			if (hasInput)
				return std::move(inputMsg);
			return std::nullopt;
		}

		std::optional<protobuf::in::Match_Pick> update(Time _diff, const protobuf::out::Match_Pick& _msg) override
		{
			bool hasInput = false;
			protobuf::in::Match_Pick inputMsg;

			// Immediately pick one factory at random
			auto playerIndex = removeDirtyFlag(_msg.playerindex());
			auto& player = _msg.player(playerIndex);
			auto& availablePicks = player.availableids();
			switch (_msg.state())
			{
				case protobuf::out::Match_Pick_State::Match_Pick_State_factoryBan:
				{
					auto& factoryBan = *inputMsg.add_bans();
					factoryBan.set_id(availablePicks.Get(randomInt(0, availablePicks.size() - 1)));
					factoryBan.set_target(protobuf::in::Match_Pick_Target_factory);
					hasInput = true;
					break;
				}
				case protobuf::out::Match_Pick_State::Match_Pick_State_abilityBan:
				{
					auto& abilityBan = *inputMsg.add_bans();
					abilityBan.set_id(availablePicks.Get(randomInt(0, availablePicks.size() - 1)));
					abilityBan.set_target(protobuf::in::Match_Pick_Target_ability);
					hasInput = true;
					break;
				}
				case protobuf::out::Match_Pick_State::Match_Pick_State_factoryPick:
				{
					auto& factoryPick = *inputMsg.add_picks();
					factoryPick.set_id(availablePicks.Get(randomInt(0, availablePicks.size() - 1)));
					factoryPick.set_target(protobuf::in::Match_Pick_Target_factory);
					hasInput = true;
					break;
				}
				case protobuf::out::Match_Pick_State::Match_Pick_State_abilityPick: 
				{
					auto& abilityPick = *inputMsg.add_picks();
					abilityPick.set_id(availablePicks.Get(randomInt(0, availablePicks.size() - 1)));
					abilityPick.set_target(protobuf::in::Match_Pick_Target_ability);
					hasInput = true;
					break;
				}
			}

			if (hasInput)
				return std::move(inputMsg);

			return std::nullopt;
		}

	private:
		int m_BotLevel = 0;
		ParticipantData m_ParticipantData;
		int m_MinReactionTime = 100;
		int m_MaxReactionTime = 5000;
		Timer m_WallBuildTimer;		// we don't want to instantly flood the map with the initial wall stack. So we use a timer to simulate
									// some reaction timings

		std::optional<TilePosition> _buildWall(Time _diff, const protobuf::out::Match_Game& _msg)
		{
			auto playerIndex = removeDirtyFlag(_msg.playerindex());
			assert(0 <= playerIndex && playerIndex < static_cast<std::uint32_t>(_msg.player_size()));

			// let's try to build some new walls
			if (!m_WallBuildTimer.isActive())
			{
				//auto buildablePos = _msg.flags() & static_cast<std::uint32_t>(entity::ParticipantProgress::settled) ? std::nullopt : generateBuildableArea(playerIndex);
				//auto positions = getBuildablePositions(_msg.buildingmap(), buildablePos);
				//if (!positions.empty())
				//{
				//	m_WallBuildTimer.start(Time(randomInt<int>(m_MinReactionTime, m_MaxReactionTime)));
				//	return positions[randomInt<std::size_t>(0, positions.size() - 1)];
				//}
			}
			else
				m_WallBuildTimer.update(_diff);
			return std::nullopt;
		}
	};
} // namespace game::ai