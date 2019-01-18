#include "stdafx.h"
#include "MinionStrategy.hpp"
#include "HeatMap.hpp"

namespace game::ai 
{

	MinionDefault::MinionDefault(int _botLevel, const StrategyFlags& _flags, const HeatMap& _heatmap, int _minionLimit) :
		m_BotLevel(_botLevel), 
		m_Flags(_flags),
		m_HeatMap(_heatmap),
		m_MinionLimit(_minionLimit)
	{
	}

	const StrategyResponse MinionDefault::update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output)
	{
		StrategyResponse response;

		// When first called, initialize the factory map
		if (m_Factories.size() == 0)
		{
			int index = 0;
			auto playerIndex = removeDirtyFlag(_output.playerindex());
			auto& player = _output.player(playerIndex);
			for (auto& factory : player.factories())
			{
				if (factory.has_minion())
				{
					MinionFactory m;
					m_Factories[index] = m;
					++index;
				}
			}
		}

		// Get the total amount of spawns left before we hit the minion limit
		auto& player = getPlayer(_output);
		auto clean = removeDirtyFlag(player.minionlimit());
		int spawnsLeft = m_MinionLimit - std::lround(m_MinionLimit * clean / 100.0);

		auto factoriesUpgrading = 0;

		// Loop over all minion factories
		int index = 0;
		for (auto& factory : player.factories())
		{
			if (factory.has_minion())
			{
				// If the factory is on cooldown, we do nothing
				if (std::chrono::milliseconds::zero() < getTimeInMSec(factory.minion().cooldown()))
				{
					++index;
					++factoriesUpgrading;
					continue;
				}

				auto totalUpgrades = getTotalUpgrades(factory.minion().upgrades());

				// Starting at level 25, we upgrade all factories twice before we do anything else
				if (25 <= m_BotLevel && totalUpgrades <= 2 && !m_Flags.hasFlag(StrategyFlagType::NoEarlyUpgrades))
				{
					*_input.add_factoryupgrades() = createFactoryUpgrade(index, totalUpgrades % 3);
					response.hasInput = true;
					++index;
					continue;
				}

				auto current = getCurrentStacks(factory.minion().stacks());
				auto max = getMaxStacks(factory.minion().stacks());


				if (0 < current)
				{
					// If we're at the limit and the exploding minion is in range, we explode it and immediately return
					if (spawnsLeft == 0)
					{
						// TODO if the next exploding minions stays the same or at the same position for too long, we explode it!
						auto foremost = player.foremostminion().guid();
						auto compareWithGuid = [](const auto& _lhs, const auto& _rhs) { return _lhs.unit().guid() < _rhs.unit().guid(); };
						assert(std::is_sorted(std::begin(player.minions()), std::end(player.minions()), compareWithGuid));
						if (auto itr = std::lower_bound(std::begin(player.minions()), std::end(player.minions()), foremost,
							[](const auto& _minion, auto _guid) { return _minion.unit().guid() < _guid; });
						itr != std::end(player.minions()) && itr->unit().guid() == foremost)
						{
							auto& minion = *itr;
							auto position = deserializeTilePos(removeDirtyFlag(minion.unit().position()));
							auto heat = m_HeatMap.heatAroundTile(HeatmapType::Damage, position);
							if (8 < heat)
							{
								*_input.add_factoryactivations() = createFactoryActivation(index);
								response.shouldReturn = true;
								return response;
							}
						}
					}

					// If we're way below the minion limit, fire at will!
					if (current < spawnsLeft)
					{
						if (m_Flags.hasFlag(StrategyFlagType::SpawnWhenReady) || current == max)
						{
							for (current = (m_BotLevel == 1 ? 1 : current); 0 < current; --current, --spawnsLeft)
							{
								*_input.add_factoryactivations() = createFactoryActivation(index);
								response.hasInput = true;
							}
							if (5 <= m_BotLevel && !m_Flags.hasFlag(StrategyFlagType::UpgradeOnlyAtMinionLimit))
							{
								if ((factoriesUpgrading + 1) < m_Factories.size())
								{
									*_input.add_factoryupgrades() = createFactoryUpgrade(index, totalUpgrades % 3);
									response.hasInput = true;
									++index;
									continue;
								}
							}
						}
					}
					else
					{
						// Spawn until we're at the minion limit
						for (; 0 < spawnsLeft; --spawnsLeft, --current)
						{
							*_input.add_factoryactivations() = createFactoryActivation(index);
							response.hasInput = true;
						}
						return response;
					}
				}
			}
			++index;
		}
		return response;
	}
}