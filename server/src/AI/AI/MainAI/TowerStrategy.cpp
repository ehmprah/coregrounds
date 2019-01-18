#include "stdafx.h"
#include "TowerStrategy.hpp"

namespace game::ai 
{

	bool isValidMoveTarget(game::TilePosition _from, game::TilePosition _to)
	{
		int xDiff = std::abs(std::abs(_from.getX()) - std::abs(_to.getX()));
		int yDiff = std::abs(std::abs(_from.getY()) - std::abs(_to.getY()));
		return (xDiff <= 2 && yDiff <= 2 && !(xDiff == 2 && yDiff == 2));
	}

	const std::optional<protobuf::out::Match_Game_Tower> getTower(const protobuf::out::Match_Game& _output, google::protobuf::uint32 _guid)
	{
		auto& player = getPlayer(_output);
		for (auto& tower : player.towers())
		{
			if (tower.unit().guid() == _guid)
			{
				return tower;
			}
		}
		return std::nullopt;
	}

	const int getAttackProgress(const protobuf::out::Match_Game& _output, google::protobuf::uint32 _guid)
	{
		auto& player = getPlayer(_output);
		for (auto& tower : player.towers())
		{
			if (tower.unit().guid() == _guid)
			{
				return removeDirtyFlag(tower.attackprogress());
			}
		}
		return 0;
	}

	const std::optional<game::TilePosition> getBuildTile(const protobuf::out::Match_Game& _output, const HeatMap& _heatmap)
	{
		auto& player = getPlayer(_output);
		if (!std::empty(player.walls()))
		{
			// Get tower positions
			std::vector<game::TilePosition> towerPositions;
			if (!player.towers().empty())
			{
				for (auto& tower : player.towers())
				{
					towerPositions.emplace_back(deserializeTilePos(tower.unit().position()));
				}
			}
			// Hold the build candidate coordinates
			int x = -1;
			int y = -1;
			// Get the threat point
			auto threat = _heatmap.getThreatPoint();
			for (auto& wall : player.walls())
			{
				auto pos = deserializeTilePos(removeDirtyFlag(wall.unit().position()));
				// Make sure we don't have a tower at that position
				if (std::find(towerPositions.begin(), towerPositions.end(), pos) == towerPositions.end())
				{
					// We have to start somewhere
					if ((x < 0 && y < 0) ||
						// We build as far west as we can while staying east of the threatline
						(pos.getX() < x && pos.getX() > threat.getX()) ||
						// We build as far south as we can while staying north of the threatline
						(pos.getY() > y && pos.getY() < threat.getY()))
					{
						x = pos.getX();
						y = pos.getY();
					}
				}
			}
			if (x >= 0 && y >= 0)
			{
				return game::TilePosition(x, y);
			}
		}
		return std::nullopt;
	}

	std::optional<TilePosition> getMoveTile(const protobuf::out::Match::Game::Player& _player, const HeatMap& _heatmap, const TilePosition& _from, const TilePosition& _to)
	{
		if (!std::empty(_player.walls()))
		{
			// Get tower positions
			std::vector<TilePosition> towerPositions;
			towerPositions.reserve(std::size(_player.towers()));
			std::transform(std::begin(_player.towers()), std::end(_player.towers()), std::back_inserter(towerPositions),
				[](const auto& _tower) { return deserializeTilePos(_tower.unit().position()); }
			);

			auto tilePosLess = [](const TilePosition& _lhs, const TilePosition& _rhs)
			{
				return _lhs.getX() < _rhs.getX() ||
					_lhs.getX() == _rhs.getX() && _lhs.getY() < _rhs.getY();
			};
			std::sort(std::begin(towerPositions), std::end(towerPositions), tilePosLess);

			// Hold the build candidate coordinates
			TilePosition movePos(-1, -1);
			// We move at least 1 distance to avoid useless jumping around
			double distance = sl2::calculateDistance(static_cast<AbsPosition>(_from), static_cast<AbsPosition>(_to)) - 0.75;
			for (auto& wall : _player.walls())
			{
				auto wallPosition = deserializeTilePos(wall.unit().position());
				// Check if wall position is within movement range
				// Make sure we don't have a tower at that position
				if (isValidMoveTarget(_from, wallPosition) &&
					std::binary_search(std::begin(towerPositions), std::end(towerPositions), wallPosition, tilePosLess))
				{
					auto distanceToTile = sl2::calculateDistance(static_cast<AbsPosition>(wallPosition), static_cast<AbsPosition>(_to));
					// Check if this is a better match than the previous
					if (distanceToTile <= distance)
					{
						movePos = wallPosition;
						distance = distanceToTile;
					}
				}
			}
			// Only return if we have found a valid move target
			if (0 <= movePos.getX() && 0 <= movePos.getY())
				return movePos;
		}
		return std::nullopt;
	}

	TowerDefault::TowerDefault(int _botlevel, const StrategyFlags& _flags, const HeatMap& _heatmap) :
		m_BotLevel(_botlevel),
		m_Flags(_flags),
		m_HeatMap(_heatmap)
	{
	}

	const StrategyResponse TowerDefault::update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output)
	{
		StrategyResponse response;

		auto playerIndex = removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);

		// When first called, initialize the factory map
		if (m_Factories.size() == 0)
		{
			int index = 0;
			bool setDefender = false;
			for (auto& factory : player.factories())
			{
				if (factory.has_tower())
				{
					TowerFactory t;
					t.id = factory.id();
					// Set defense role for ONE tower (with a few exceptions)
					if (!setDefender && t.id != 1115)
					{
						t.defense = true;
						setDefender = true;
					}
					m_Factories[index] = t;
				}
				++index;
			}
		}

		// TODO if core < 50% and there's heat around it, retreat everything regardless!
		bool retreatAllTowers = false;
		for (int index = 0; index < std::size(player.factories()); ++index)
		{
			auto& factory = player.factories(index);
			if (factory.has_tower())
			{
				// If the factory is on cooldown, we do nothing
				if (std::chrono::milliseconds::zero() < getTimeInMSec(factory.tower().cooldown()))
				{
					++index;
					continue;
				}

				auto totalUpgrades = getTotalUpgrades(factory.tower().upgrades());

				// Starting at level 25, we upgrade all towers twice before we do anything else
				if (25 <= m_BotLevel && totalUpgrades <= 2 && !m_Flags.hasFlag(StrategyFlagType::NoEarlyUpgrades))
				{
					if (!m_Flags.hasFlag(StrategyFlagType::BuildTowersAsap))
					{
						*_input.add_factoryupgrades() = createFactoryUpgrade(index, totalUpgrades % 3);
						response.shouldReturn = true;
						return response;
					}
				}

				// First we try to build the tower
				if (factory.tower().guid() == 0)
				{
					if (auto pos = getBuildTile(_output, m_HeatMap))
					{
						*_input.add_factoryactivations() = createFactoryActivation(index, pos.value());
						response.shouldReturn = true;
						return response;
					}
				}

				// For the following stuff we need the tower unit
				if (auto optTower = getTower(_output, factory.tower().guid()))
				{
					auto& tower = *optTower;
					auto position = deserializeTilePos(tower.unit().position());

					int action = TowerActions::None;
					TilePosition moveTo;

					// If we're below 20% HP we move backwards until we're at 0 heat and upgrade 
					if (removeDirtyFlag(tower.unit().hp()) <= 20)
					{
						if (m_HeatMap.heatAroundTile(HeatmapType::Damage, position, 2) == 0)
						{
							action = TowerActions::Upgrade;
						}
						else
						{
							if (auto pos = getMoveTile(player, m_HeatMap, position, { 14, 0 }))
							{
								action = TowerActions::Move;
								moveTo = *pos;
							}
						}
					}

					// If this is the defender, we keep it close to the threatpoint
					if (action == TowerActions::None)
					{
						if (m_Factories[index].defense)
						{
							if (m_Factories[index].lastDefenseMove > _output.gametime())
							{
								auto threat = m_HeatMap.getThreatPoint();
								// We move back if the threat has moved past the defense tower
								if (threat.getX() > position.getX() && 2 < sl2::calculateDistance(threat, position))
								{
									if (auto pos = getMoveTile(player, m_HeatMap, position, threat))
									{
										action = TowerActions::Move;
										moveTo = *pos;
										m_Factories[index].lastDefenseMove = _output.gametime() + 5000;
									}
								}
							}

						}
					}

					// If rushing, we don't wait until our towers are idle
					if (m_Flags.hasFlag(StrategyFlagType::MoveTowersToCore))
					{
						action = TowerActions::MoveOrUpgrade;
					}

					// Check if the tower is idle
					if (action == TowerActions::None)
					{
						if (removeDirtyFlag(tower.attackprogress()) == 100)
						{
							// We allow the tower to be idle for one tick
							if (!std::exchange(m_Factories[index].idle, true))
								continue;
							// Otherwise we'll move forward or upgrade
							else 
							{
								action = TowerActions::MoveOrUpgrade; 
								if (removeDirtyFlag(tower.unit().hp()) <= 80 ||
									(m_Flags.hasFlag(StrategyFlagType::EarlyLateGame) && totalUpgrades < 6) ||
									(m_Flags.hasFlag(StrategyFlagType::EarlyMidGame) && totalUpgrades < 4))
									action = TowerActions::Upgrade; 
							}
						}
					}

					// At last we check whether we're in a spot without heat for a long time
					if (action == TowerActions::None)
					{
						if (m_HeatMap.heatAroundTile(HeatmapType::Damage, position) == 0)
						{
							if (m_Factories[index].heatLessTime == 0)
							{
								m_Factories[index].heatLessTime = _output.gametime();
								continue;
							}
							else if (m_Factories[index].heatLessTime + 30000 < _output.gametime())
							{
								if (auto optTo = m_HeatMap.getHotTile(HeatmapType::Damage))
								{
									action = TowerActions::Move;
									moveTo = *optTo;
								}
							}
						}
						else 
						{
							m_Factories[index].heatLessTime = 0;
						}
					}

					if (action == TowerActions::MoveOrUpgrade)
					{
						// Move towards core or heat depending on flag
						auto to = m_Flags.hasFlag(StrategyFlagType::MoveTowersToCore) ?
							TilePosition(0, 6) :
							m_HeatMap.getHotTile(HeatmapType::Damage).value_or(TilePosition(0, 6));
						if (auto pos = getMoveTile(player, m_HeatMap, position, to))
						{
							action = TowerActions::Move;
							moveTo = pos.value();
						}
						else
						{
							// We only upgrade if there's no heat within 2 tiles of the position
							if (m_HeatMap.heatAroundTile(HeatmapType::Damage, position, 2) == 0)
							{
								action = TowerActions::Upgrade;
							}
							else 
							{
								// This is an ok scenario, even a good one maybe. We're in place, but can't attack at the moment.
								// We need a safeguard so this doesn't take too long, though.
							}
						}
					}

					if (action == TowerActions::Move)
					{
						*_input.add_factoryactivations() = createFactoryActivation(index, moveTo);
						response.shouldReturn = true;
						return response;
					}

					if (5 <= m_BotLevel && action == TowerActions::Upgrade)
					{
						*_input.add_factoryupgrades() = createFactoryUpgrade(index, totalUpgrades % 3);
						response.shouldReturn = true;
						return response;
					}
				}
			}
		}
		return response;
	}
}