#pragma once

#include "Core/StdInclude.hpp"
#include "Core/SharedDefines.hpp"

namespace game::data
{
	struct GameProperties;
	class Game;

	struct Ability;
	struct Shape;
	struct Aura;
	struct Device;
	struct Factory;
	struct Spell;

	struct Unit;
	struct ActiveUnit;
	struct Core;
	struct Wall;
	struct Tower;
	struct Minion;
	struct Projectile;
	struct Trigger;

	struct Color;
	struct Emote;
	struct Modification;

	struct UpgradeDuration;
	using UpgradeDurations = std::vector<UpgradeDuration>;

	using XPCurve = std::vector<Stat>;

	template <class T>
	using Table = std::vector<T>;
} // namespace game::data
