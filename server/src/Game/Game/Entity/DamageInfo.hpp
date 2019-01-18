#pragma once

#include "LifeLink.hpp"

namespace game::unit
{
	struct DamageInfo
	{
		LifeLinkPool lifeLinkPool;			// unique life pool units; sorted by there ptr value

		Stat plainDamage = 0;				// damage before all modifications
		Stat damageAfterArmor = 0;			// plainDamage modified by armor
		Stat damageAfterDmgTaken = 0;		// armor dmg modified by dmg taken stat
		Stat damageAfterModifiers = 0;		// damageAfterArmor modified
		Stat absorbedDamage = 0;			// absorbed dmg from auras; can not be greater than damgaeAfterModfifiers
		Stat totalDamage = 0;				// final damage inclusive life link damage
		Stat sharedDamage = 0;				// damage shared between other life link member; can not be greater than totalDamage
		Stat finalDamage = 0;				// final damage dealt to unit

		Stat hpLeeched = 0;					// hp for dealer
		Stat thornsDamage = 0;				// damage to dealer
	};
} // namespace game::unit
