#include "DataStringHelper.hpp"

namespace game::data
{
	device::AbilityFlag parseAbilityFlag(std::string_view _flagName)
	{
		using device::AbilityFlag;
		if (_flagName == "require_unit_target")
			return AbilityFlag::requireUnitTarget;
		return AbilityFlag::none;
	}

	spell::EffectType game::data::parseSpellEffectType(std::string_view _effectName)
	{
		using spell::EffectType;
		if (_effectName == "apply_aura")
			return EffectType::applyAura;
		else if (_effectName == "damage")
			return EffectType::damage;
		else if (_effectName == "healing")
			return EffectType::healing;
		else if (_effectName == "instant_kill")
			return EffectType::instantKill;
		else if (_effectName == "health_as_damage")
			return EffectType::healthAsDamage;
		else if (_effectName == "trigger_spell")
			return EffectType::triggerSpell;
		else if (_effectName == "spawn_minion")
			return EffectType::spawnMinion;
		else if (_effectName == "spawn_trigger")
			return EffectType::spawnTrigger;
		else if (_effectName == "change_owner")
			return EffectType::changeOwner;
		return EffectType::none;
	}

	unit::targetPreference::Mode parseTargetPreferenceMode(std::string_view _modeName)
	{
		using unit::targetPreference::Mode;
		if (_modeName == "high")
			return Mode::high;
		else if (_modeName == "low")
			return Mode::low;
		else if (_modeName == "first")
			return Mode::first;
		else if (_modeName == "random")
			return Mode::random;
		return Mode::none;
	}

	unit::targetPreference::Target parseTargetPreferenceTarget(std::string_view _targetName)
	{
		using unit::targetPreference::Target;
		if (_targetName == "core")
			return Target::core;
		else if (_targetName == "wall")
			return Target::wall;
		else if (_targetName == "tower")
			return Target::tower;
		else if (_targetName == "minion")
			return Target::minion;
		else if (_targetName == "all")
			return Target::all;
		return Target::none;
	}

	unit::targetPreference::Flag parseTargetPreferenceFlag(std::string_view _string)
	{
		using unit::targetPreference::Flag;
		if (_string == "damaged")
			return Flag::damaged;
		else if (_string == "all")
			return Flag::all;
		return Flag::none;
	}

	spell::TargetFlags parseSpellTargetFlag(std::string_view _flagName)
	{
		using spell::TargetFlags;
		if (_flagName == "unit")
			return TargetFlags::unit;
		else if (_flagName == "core")
			return TargetFlags::core;
		else if (_flagName == "wall")
			return TargetFlags::wall;
		else if (_flagName == "minion")
			return TargetFlags::minion;
		else if (_flagName == "tower")
			return TargetFlags::tower;
		else if (_flagName == "device")
			return TargetFlags::device;
		else if (_flagName == "factory")
			return TargetFlags::factory;
		else if (_flagName == "minion_factory")
			return TargetFlags::minion_factory;
		else if (_flagName == "tower_factory")
			return TargetFlags::tower_factory;
		else if (_flagName == "wall_factory")
			return TargetFlags::wall_factory;
		else if (_flagName == "ability")
			return TargetFlags::ability;
		else if (_flagName == "all")
			return TargetFlags::all;
		return TargetFlags::none;
	}

	spell::SpellFlags parseSpellFlag(std::string_view _flagName)
	{
		using Flags = spell::SpellFlags;
		if (_flagName == "self_cast")
			return Flags::selfCast;
		else if (_flagName == "caster_no_target")
			return Flags::casterNoTarget;
		else if (_flagName == "tile_aligned")
			return Flags::tileAligned;
		else if (_flagName == "unit_range_as_area_size")
			return Flags::unitRangeAsAreaSize;
		return Flags::none;
	}

	aura::AuraFlags parseAuraFlag(std::string_view _flagName)
	{
		using Flags = aura::AuraFlags;
		if (_flagName == "individual_caster_stack")
			return Flags::individualCasterStack;
		else if (_flagName == "accumulate_duration")
			return Flags::accumulateDuration;
		return Flags::none;
	}

	aura::EffectType parseAuraEffectType(std::string_view _effectName)
	{
		using aura::EffectType;
		if (_effectName == "mod_stat")
			return EffectType::modStat;
		else if (_effectName == "mod_inc_spell_effectivity")
			return EffectType::modIncSpell;
		else if (_effectName == "mod_stat_percent")
			return EffectType::modStatPercent;
		else if (_effectName == "apply_condition")
			return EffectType::condition;
		else if (_effectName == "trigger_spell")
			return EffectType::triggerSpell;
		else if (_effectName == "periodic_trigger_spell")
			return EffectType::periodicTrigger;
		else if (_effectName == "life_link")
			return EffectType::lifeLink;
		else if (_effectName == "mod_spell")
			return EffectType::modSpell;
		else if (_effectName == "offender")
			return EffectType::offender;
		else if (_effectName == "dummy")
			return EffectType::dummy;
		else if (_effectName == "periodic_damage")
			return EffectType::periodicDamage;
		else if (_effectName == "periodic_damage_percent")
			return EffectType::periodicDamagePercent;
		else if (_effectName == "periodic_healing")
			return EffectType::periodicHealing;
		else if (_effectName == "periodic_healing_percent")
			return EffectType::periodicHealingPercent;
		return EffectType::none;
	}

	aura::TriggerType parseAuraTriggerType(std::string_view _triggerName)
	{
		using aura::TriggerType;
		if (_triggerName == "died")
			return TriggerType::died;
		if (_triggerName == "summoned_unit")
			return TriggerType::summonedUnit;
		return TriggerType::none;
	}

	StatModifierGroup parseEntityStatModifierGroupe(std::string_view _name)
	{
		using Grp = StatModifierGroup;
		if (_name == "modification")
			return Grp::modification;
		if (_name == "upgrade")
			return Grp::upgrade;
		if (_name == "buff")
			return Grp::buff;
		return Grp::none;
	}

	unit::StatType parseUnitStatType(std::string_view _effectName)
	{
		using unit::StatType;
		static const auto max = static_cast<std::size_t>(StatType::max);
		for (std::size_t i = 1; i < max; ++i)
		{
			auto type = static_cast<StatType>(i);
			if (_effectName == parseStatTypeToString(type))
				return type;
		}
		return StatType::none;
	}

	unit::Condition parseUnitCondition(std::string_view _conditionName)
	{
		using unit::Condition;
		if (_conditionName == "root")
			return Condition::root;
		if (_conditionName == "daze")
			return Condition::daze;
		if (_conditionName == "invulnerable")
			return Condition::invulnerable;
		if (_conditionName == "silence")
			return Condition::silence;
		return Condition::none;
	}

	std::string_view parseStatTypeToString(unit::StatType _type)
	{
		using unit::StatType;
		switch (_type)
		{
		case StatType::armor:				return "armor";
		case StatType::armorPenetration:	return "armor_pen";
		case StatType::attackRange:			return "attack_range";
		case StatType::attackSpeed:			return "attack_speed";
		case StatType::damage:				return "damage";
		case StatType::damageTaken:			return "damage_taken";
		case StatType::healing:				return "healing";
		case StatType::maxHealth:			return "max_health";
		case StatType::moveSpeed:			return "movement_speed";
		case StatType::regeneration:		return "regeneration";
		case StatType::thorns:				return "thorns";
		case StatType::visionRange:			return "vision_range";
		}
		return "none";
	}

	unit::MinionFlag parseMinionFlag(std::string_view _name)
	{
		using Flag = unit::MinionFlag;
		if (_name == "unsolid")
			return Flag::unsolid;
		return Flag::none;
	}

	unit::Modifier parseUnitModifier(std::string_view _name)
	{
		using unit::Modifier;
		static const auto max = static_cast<std::size_t>(Modifier::max);
		for (std::size_t i = 1; i < max; ++i)
		{
			auto type = static_cast<Modifier>(i);
			if (_name == parseUnitModifierToString(type))
				return type;
		}
		return Modifier::none;
	}

	std::string_view parseUnitModifierToString(unit::Modifier _mod)
	{
		using unit::Modifier;
		switch (_mod)
		{
		case Modifier::xpOnDamageTaken:			return "xp_damage_taken";
		case Modifier::xpOnDamageDealt:			return "xp_damage_dealt";
		case Modifier::xpOnUnitKilled:			return "xp_unit_killed";
		case Modifier::xpOnHealingDealt:		return "xp_healing_dealt";
		case Modifier::damageAgainstCores:		return "damage_against_core";
		case Modifier::damageAgainstMinions:	return "damage_against_minion";
		case Modifier::damageAgainstTowers:		return "damage_against_tower";
		case Modifier::damageAgainstWalls:		return "damage_against_wall";
		case Modifier::damageAgainstCC:			return "damage_against_crowd_control";
		case Modifier::healGained:				return "heal_gained";
		}
		return "none";
	}

	device::StatType parseDeviceStatType(std::string_view _effectName)
	{
		using device::StatType;
		static const auto max = static_cast<std::size_t>(StatType::max);
		for (std::size_t i = 1; i < max; ++i)
		{
			auto type = static_cast<StatType>(i);
			if (_effectName == parseStatTypeToString(type))
				return type;
		}
		return StatType::none;
	}

	std::string_view parseStatTypeToString(device::StatType _type)
	{
		using device::StatType;
		switch (_type)
		{
		case StatType::stacks:			return "stacks";
		case StatType::spawnSpeed:		return "spawn_speed";
		case StatType::xp:				return "xp";
		case StatType::cooldownSpeed:	return "cooldown_speed";
		}
		return "none";
	}

	unit::TriggerFlag parseTriggerFlag(std::string_view _name)
	{
		using unit::TriggerFlag;
		if (_name == "modify_visibility")
			return TriggerFlag::modifyVisibility;
		return TriggerFlag::none;
	}

	unit::ProjectileFlag parseProjectileFlag(std::string_view _name)
	{
		using unit::ProjectileFlag;
		if (_name == "unique_hit")
			return ProjectileFlag::uniqueHit;
		return ProjectileFlag::none;
	}
} // namespace game::data
