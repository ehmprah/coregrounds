#pragma once

#include "AI\Utility.hpp"
#include "StrategyResponse.hpp"
#include "HeatMap.hpp"
#include "StrategyFlags.hpp"

namespace game::ai 
{
	
	enum TowerActions 
	{
		None,
		Move,
		Upgrade,
		MoveOrUpgrade,
	};

	struct TowerFactory 
	{
		int id = 0;
		bool defense = false;
		bool idle = false;
		google::protobuf::uint32 heatLessTime = 0;
		google::protobuf::uint32 lastDefenseMove = 0;
	};

	class TowerStrategy
	{
		public:
			virtual ~TowerStrategy() = default;
			virtual const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) = 0;
	};

	class TowerDefault : public TowerStrategy
	{
		public:
			TowerDefault(int _botlevel, const StrategyFlags& _flags, const HeatMap& _heatmap);
			const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) override;

		private:
			const int m_BotLevel;
			std::map<int, TowerFactory> m_Factories;
			const StrategyFlags& m_Flags;
			const HeatMap& m_HeatMap;
	};
}