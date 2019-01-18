#include "Game.hpp"
#include "Core/log.hpp"
#include "Spell.hpp"
#include "Aura.hpp"
#include "Unit.hpp"
#include "Factory.hpp"
#include "Ability.hpp"

namespace game::data
{
	const Size<TileCoordinateType> GameProperties::Game::fieldSize(Vector2D<TileCoordinateType>(15, 7));

	Error::Error(const std::string& _what) :
		super(_what)
	{
	}

	void GameProperties::_checkAndAdjustUpgradeDurations()
	{
		//if (game.upgradeDurations.size() < device::MaxFactoryUpgrades * device::MaxFactoryUpgradeTypes)
		//	LOG_WARN("GameProperties: upgrade_durations: not enough elements");
		//float lastTime = 0;
		//for (std::size_t i = 0; i < game.upgradeDurations.size(); ++i)
		//{
		//	auto& curTime = game.upgradeDurations[i].time;
		//	if (curTime <= 0)
		//		throw Error("GameProperties: upgrade_durations index: " + std::to_string(i) + " must be greater than 0.");
		//	else if (curTime <= lastTime)
		//		throw Error("GameProperties: upgrade_durations index: " + std::to_string(i) + " is less than or equal previous duration.");
		//	lastTime = curTime;
		//}
	}

	void GameProperties::_checkAndAdjustXPCurve()
	{
		if (game.xpCurve.size() != *game.maxLevel)
			throw Error("GameProperties: xp_curve size must be equal to max_unit_level.");

		if (!std::is_sorted(std::begin(game.xpCurve), std::end(game.xpCurve), std::greater<Stat>()))
			throw Error("GameProperties: xp_curve index: seems values are not sorted (logic error?)");
	}

	void GameProperties::checkAndAdjust()
	{
		_checkAndAdjustUpgradeDurations();
		_checkAndAdjustXPCurve();
	}

	void Game::checkAndAdjust()
	{
	}
} // namespace game::data
