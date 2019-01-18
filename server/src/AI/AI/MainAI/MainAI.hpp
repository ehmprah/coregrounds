#pragma once

#include "AI\BaseAI.hpp"
#include "AI\Utility.hpp"
#include "Core\Timer.hpp"
#include "WallStrategy.hpp"
#include "AbilityStrategy.hpp"
#include "MinionStrategy.hpp"
#include "TowerStrategy.hpp"
#include "StrategyFlags.hpp"

namespace game::ai 
{
	enum EmoteIds {
		Hello				= 2202,
		GoodGame			= 2201,
		GoodLuckHaveFun		= 2218,
	};

	struct EmoteActivations 
	{
		bool hello = false;
		bool glhf = false;
		bool gg = false;
	};

	class MainAI : public BaseAI
	{
		private:
			int m_BotLevel;
			int m_ReactionOffset;
			Timer m_ReactionTimer;
			EmoteActivations m_EmoteActivations;
			const data::AiStrategy& m_Strategy;
			const data::AiMazeLayout& m_Layout;
			ParticipantData m_ParticipantData;

			HeatMap m_HeatMap;
			StrategyFlags m_StrategyFlags;

			std::unique_ptr<WallStrategy> m_WallStrategy;
			std::unique_ptr<MinionStrategy> m_MinionStrategy;
			std::unique_ptr<TowerStrategy> m_TowerStrategy;
			std::unique_ptr<AbilityStrategy> m_AbilityStrategy;

		public:
			MainAI(const Initializer& _initializer);
			std::optional<protobuf::in::Match_Game> update(Time _diff, const protobuf::out::Match_Game& _msg) override;
			std::optional<protobuf::in::Match_Pick> update(Time _diff, const protobuf::out::Match_Pick& _msg) override;
			std::optional<protobuf::in::Match::Finalize> update(Time _diff, const protobuf::out::Match::Finalize& _msg) override;
			const ParticipantData& getParticipantData() const override;
	};
}