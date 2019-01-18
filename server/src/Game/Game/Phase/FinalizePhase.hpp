#pragma once

#include "PhaseInterface.hpp"
#include "Game/Entity/Participant.hpp"
#include "Data/Game.hpp"
#include "GameStatistics.hpp"

namespace game::phase
{
	class Finalize :
		public Interface
	{
		using super = Interface;
	public:
		Finalize(const std::vector<Team>& _teams, const data::Game& _gameData);

		void setup(Statistics _statistics, std::vector<FinalizeParticipantPtr> _participants);

		const std::vector<FinalizeParticipantPtr>& getParticipants() const;

		std::optional<ID> getPermanentMatchId() const;
		std::optional<int> getWinningTeamId() const;

		Data update(Time _diff) override;
		bool hasFinished() const override;

		virtual void start() override {};
		virtual void finalize() override {};

		virtual Type getType() const override;

		virtual VisualCollector& getVisualCollector() override;
		virtual const VisualCollector& getVisualCollector() const override;

		const Timer& getPhaseTimer() const;

	private:
		const data::Game& m_GameData;
		VisualCollector m_VisualCollector;
		std::optional<Statistics> m_MatchStatistics;
		std::optional<ID> m_PermanentMatchId;
		Timer m_PhaseTimer;

		std::vector<FinalizeParticipantPtr> m_Participants;
		const std::vector<Team>& m_Teams;
	};
	using FinalizePtr = std::unique_ptr<Finalize>;
} // namespace game::phase