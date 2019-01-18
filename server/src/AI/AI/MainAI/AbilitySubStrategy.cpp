#include "stdafx.h"
#include "AbilitySubStrategy.hpp"

namespace game::ai 
{
	AbilitySubStrategy::AbilitySubStrategy(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap) :
		m_Flags(_strategyFlags),
		m_HeatMap(_heatMap),
		m_BotLvl(_botLvl),
		m_Id(_id)
	{
	}

	const StrategyFlags& AbilitySubStrategy::getStrategyFlags() const
	{
		return m_Flags;
	}

	const HeatMap& AbilitySubStrategy::getHeatMap() const
	{
		return m_HeatMap;
	}

	int AbilitySubStrategy::getBotLevel() const
	{
		return m_BotLvl;
	}

	ID AbilitySubStrategy::getId() const
	{
		return m_Id;
	}

	AbilitySubStrategyAutoActivate::AbilitySubStrategyAutoActivate(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap) :
		super(_id, _botLvl, _strategyFlags, _heatMap)
	{
	}

	std::optional<AbsPosition> AbilitySubStrategyAutoActivate::update(const protobuf::out::Match_Game& _output)
	{
		return { {0, 0} };
	}

	AbilitySubStrategyIonstrike::AbilitySubStrategyIonstrike(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap) :
		super(_id, _botLvl, _strategyFlags, _heatMap)
	{
	}

	std::optional<AbsPosition> AbilitySubStrategyIonstrike::update(const protobuf::out::Match_Game& _output)
	{
		auto& opponent = getOpponent(_output);
		if (std::empty(opponent.towers()))
			return std::nullopt;

		// Below level 25 we just aim at towers
		if (getBotLevel() < 25)
		{
			auto& tower = getRandomElement(opponent.towers());
			return getPosition(tower.unit());
		}
		// Above that, we aim at towers on cooldown > 5 sec
		else
		{
			for (auto& tower : opponent.towers())
			{
				if (std::chrono::seconds(5) <= getTimeInSeconds(tower.cooldown()))
				{
					auto& tower = getRandomElement(opponent.towers());
					return getPosition(tower.unit());
				}
			}
		}
		return std::nullopt;
	}

	AbilitySubStrategyActivateOnHotspot::AbilitySubStrategyActivateOnHotspot(HeatmapType _heatmap, int _threshold, ID _id, int _botLvl,
		const StrategyFlags& _strategyFlags, const HeatMap& _heatMap) :
		super(_id, _botLvl, _strategyFlags, _heatMap),
		m_HeatmapType(_heatmap),
		m_Threshold(_threshold)
	{
	}

	std::optional<AbsPosition> AbilitySubStrategyActivateOnHotspot::update(const protobuf::out::Match_Game& _output)
	{
		if (auto opt = getHeatMap().getHotSpot(m_HeatmapType, m_Threshold))
		{
			auto position = jitterAbsPos(*opt);
			if (getId() == AbilityID::Explode)
				position += 0.5;
			return position;
		}
		return std::nullopt;
	}

	AbilitySubStrategyWall::AbilitySubStrategyWall(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap) :
		super(_id, _botLvl, _strategyFlags, _heatMap)
	{
	}

	std::optional<game::AbsPosition> AbilitySubStrategyWall::update(const protobuf::out::Match_Game& _output)
	{
		auto& player = getPlayer(_output);
		auto& opponent = getOpponent(_output);
		if (!std::empty(opponent.walls()))
		{
			auto to = getStrategyFlags().hasFlag(StrategyFlagType::DestroyWallsOffensively)
				? getPosition(opponent.core().unit())
				: getPosition(player.core().unit());

			double distance = 10.0;
			game::AbsPosition target;

			for (auto& wall : opponent.walls())
			{
				auto from = deserializeAbsPos(wall.unit().position());
				auto newDistance = sl2::calculateDistance(to, from);
				if (newDistance < distance)
				{
					target = from;
					distance = newDistance;
				}
			}
			return target;
		}
		return std::nullopt;
	}

	AbilitySubStrategyPtr makeAbilitySubStrategy(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap)
	{
		switch (_id)
		{
		case Ionstrike:
			return std::make_unique<AbilitySubStrategyIonstrike>(_id, _botLvl, _strategyFlags, _heatMap);
		case Demolish:
		case Wallswap:
			return std::make_unique<AbilitySubStrategyWall>(_id, _botLvl, _strategyFlags, _heatMap);
		case MassProduction:
			return std::make_unique<AbilitySubStrategyAutoActivate>(_id, _botLvl, _strategyFlags, _heatMap);
		case Explode:
		case Stun:
			return std::make_unique<AbilitySubStrategyActivateOnHotspot>(HeatmapType::Damage, 1, _id, _botLvl, _strategyFlags, _heatMap);
		case Heal:
		case Adrenaline:
		case Shield:
		case Frenzy:
		case Lifelink:
			return std::make_unique<AbilitySubStrategyActivateOnHotspot>(HeatmapType::Healing, 3, _id, _botLvl, _strategyFlags, _heatMap);
		}
		return std::make_unique<AbilitySubStrategyAutoActivate>(_id, _botLvl, _strategyFlags, _heatMap);
	}
} // namespace game::ai
