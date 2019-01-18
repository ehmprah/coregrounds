#pragma once

#include "AI\Utility.hpp"
#include "StrategyResponse.hpp"
#include "HeatMap.hpp"
#include "StrategyFlags.hpp"

namespace game::ai 
{
	enum AbilityID
	{
		Heal			= 1400,
		Explode			= 1401,
		Demolish		= 1402,
		Ionstrike		= 1403,
		Farsight		= 1404,
		Hurry			= 1405,
		Slow			= 1406,
		Frenzy			= 1407,
		Shield			= 1408,
		Stun			= 1409,
		MassProduction	= 1410,
		Adrenaline		= 1411,
		Teleport		= 1412,
		Lifelink		= 1413,
		Wallswap		= 1414,
	};

	class AbilitySubStrategy
	{
	public:
		AbilitySubStrategy(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);

		virtual ~AbilitySubStrategy() = default;

		virtual std::optional<AbsPosition> update(const protobuf::out::Match_Game& _output) = 0;

		const StrategyFlags& getStrategyFlags() const;
		const HeatMap& getHeatMap() const;
		int getBotLevel() const;
		ID getId() const;

	private:
		const StrategyFlags& m_Flags;
		const HeatMap& m_HeatMap;
		int m_BotLvl;
		ID m_Id;
	};

	class AbilitySubStrategyAutoActivate :
		public AbilitySubStrategy
	{
		using super = AbilitySubStrategy;

	public:
		AbilitySubStrategyAutoActivate(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);

		virtual std::optional<AbsPosition> update(const protobuf::out::Match_Game& _output) override;
	};

	class AbilitySubStrategyIonstrike :
		public AbilitySubStrategy
	{
		using super = AbilitySubStrategy;

	public:
		AbilitySubStrategyIonstrike(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);

		virtual std::optional<AbsPosition> update(const protobuf::out::Match_Game& _output) override;
	};

	class AbilitySubStrategyActivateOnHotspot :
		public AbilitySubStrategy
	{
		using super = AbilitySubStrategy;

	public:
		AbilitySubStrategyActivateOnHotspot(HeatmapType _heatmap, int _threshold, ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);

		virtual std::optional<AbsPosition> update(const protobuf::out::Match_Game& _output) override;

	private:
		HeatmapType m_HeatmapType;
		int m_Threshold;
	};

	class AbilitySubStrategyWall :
		public AbilitySubStrategy
	{
		using super = AbilitySubStrategy;

	public:
		AbilitySubStrategyWall(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);

		virtual std::optional<AbsPosition> update(const protobuf::out::Match_Game& _output) override;
	};

	using AbilitySubStrategyPtr = std::unique_ptr<AbilitySubStrategy>;
	AbilitySubStrategyPtr makeAbilitySubStrategy(ID _id, int _botLvl, const StrategyFlags& _strategyFlags, const HeatMap& _heatMap);
} // namespace game::ai
