#pragma once

#include "AI\Utility.hpp"
#include "StrategyResponse.hpp"
#include "HeatMap.hpp"
#include "StrategyFlags.hpp"

namespace game::ai 
{

	struct MinionFactory 
	{
		int id = 0;
		int idleTime = false;
	};

	class MinionStrategy
	{
	public:
		virtual ~MinionStrategy() = default;
		virtual const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) = 0;
	};

	class MinionDefault : public MinionStrategy
	{
	public:
		MinionDefault(int _botLevel, const StrategyFlags& _flags, const HeatMap& _heatmap, int _minionLimit);
		const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) override;

	private:
		int m_BotLevel;
		int m_MinionLimit;
		std::map<int, MinionFactory> m_Factories;
		const StrategyFlags& m_Flags;
		const HeatMap& m_HeatMap;
	};
}