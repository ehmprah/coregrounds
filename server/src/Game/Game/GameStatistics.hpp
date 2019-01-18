#pragma once

#include "Entity/Participant.hpp"

namespace game {

	struct Statistics
	{
		ID tempMatchId = 0;
		std::optional<ID> winnerTeam;				// invalid if draw
		std::chrono::seconds duration;
		std::string matchType;
		std::vector<Participant::Statistics> players;

		json::Object toJson() const;
	};
} // namespace game
