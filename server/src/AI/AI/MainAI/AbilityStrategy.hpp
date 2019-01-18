#pragma once

#include "AbilitySubStrategy.hpp"

namespace game::ai 
{
	class AbilityStrategy
	{
	public:
		virtual ~AbilityStrategy() = default;
		virtual StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) = 0;
	};
	
	class AbilityDefault : public AbilityStrategy
	{
	public:
		AbilityDefault(int _botlevel, const StrategyFlags& _flags, const HeatMap& _heatmap);
		StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) override;
			
	private:
		int m_BotLevel;
		int m_Downtime;
		const StrategyFlags& m_Flags;
		const HeatMap& m_HeatMap;

		std::array<AbilitySubStrategyPtr, 3> m_SubStrategies;
	};
}