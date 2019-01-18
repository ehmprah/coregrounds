#pragma once

#include "StdInclude.hpp"
#include "SLInclude.hpp"
#include "UnitFwd.hpp"

namespace game::unit
{
	using Towers = std::vector<Tower*>;
	using Minions = std::vector<Minion*>;
	using Walls = std::vector<Wall*>;
	using Triggers = std::vector<Trigger*>;
	using Projectiles = std::vector<Projectile*>;

	using UnitHandle = sl2::Handle<Unit>;
	using TowerHandle = sl2::Handle<Tower>;
	using WallHandle = sl2::Handle<Wall>;
	using ProjectileHandle = sl2::Handle<Projectile>;

	using GUID = std::uint32_t;

	enum class Direction
	{
		none = 0x00,
		up = 0x01,
		right = 0x02,
		down = 0x04,
		left = 0x08
	};
} // namespace game::unit
