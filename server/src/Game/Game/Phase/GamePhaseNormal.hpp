#pragma once

#include "AbstractGamePhase.hpp"

namespace game::phase
{
	class GameNormal :
		public AbstractGame
	{
	private:
		using super = AbstractGame;

	public:
		GameNormal(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _gameData);

		Data update(Time _diff) override;
	};
} // namespace game::phase
