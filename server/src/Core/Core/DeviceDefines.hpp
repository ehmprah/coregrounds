#pragma once

#include "StdInclude.hpp"
#include "DeviceFwd.hpp"

namespace game::device
{
	using Abilities = std::array<Ability*, MAX_ABILITIES>;
	using Factories = std::array<Factory*, MAX_FACTORIES>;
	using FactoryUpgrades = std::array<int, MaxFactoryUpgradeTypes>;
} // namespace game::device
