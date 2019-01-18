#include "stdafx.h"
#include "GamePhaseNormal.hpp"

namespace game::phase
{
	GameNormal::GameNormal(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _gameData) :
		super(_teams, _gameData)
	{
	}

	Data GameNormal::update(Time _diff)
	{
		return super::update(_diff);
	}
} // namespace game::phase
