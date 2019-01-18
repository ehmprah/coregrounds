#include "stdafx.h"
#include "FinalizePhase.hpp"

namespace game::phase
{
	Finalize::Finalize(const std::vector<Team>& _teams, const data::Game& _gameData) :
		m_Teams(_teams),
		m_GameData(_gameData)
	{
		m_PhaseTimer.start(*m_GameData.gameProperties.finalize.phaseTime);
	}

	void Finalize::setup(Statistics _statistics, std::vector<FinalizeParticipantPtr> _participants)
	{
		m_MatchStatistics = std::move(_statistics);
		m_Participants = std::move(_participants);
		m_VisualCollector.resize(m_Participants.size());
	}

	const std::vector<FinalizeParticipantPtr>& Finalize::getParticipants() const
	{
		return m_Participants;
	}

	std::optional<ID> Finalize::getPermanentMatchId() const
	{
		return m_PermanentMatchId;
	}

	Data Finalize::update(Time _diff)
	{
		assert(m_MatchStatistics);

		m_PhaseTimer.update(_diff);
		m_VisualCollector.swap();
		return FinalizeState{ &*m_MatchStatistics, &m_PermanentMatchId };
	}

	bool Finalize::hasFinished() const
	{
		return !m_PhaseTimer.isActive();
	}

	VisualCollector& Finalize::getVisualCollector()
	{
		return m_VisualCollector;
	}

	const VisualCollector& Finalize::getVisualCollector() const
	{
		return m_VisualCollector;
	}

	Type Finalize::getType() const
	{
		return Type::finalize;
	}

	const Timer& Finalize::getPhaseTimer() const
	{
		return m_PhaseTimer;
	}

	std::optional<int> Finalize::getWinningTeamId() const
	{
		if (m_MatchStatistics)
			return m_MatchStatistics->winnerTeam;
		return std::nullopt;
	}
} // namespace game::phase
