#pragma once

#include <string>
#include "Core/SharedDefines.hpp"

namespace game::data
{
	device::AbilityFlag parseAbilityFlag(std::string_view _flagName);

	unit::targetPreference::Mode parseTargetPreferenceMode(std::string_view _modeName);
	unit::targetPreference::Target parseTargetPreferenceTarget(std::string_view _targetName);
	unit::targetPreference::Flag parseTargetPreferenceFlag(std::string_view _targetName);

	spell::EffectType parseSpellEffectType(std::string_view _effectName);
	spell::TargetFlags parseSpellTargetFlag(std::string_view _flagName);
	spell::SpellFlags parseSpellFlag(std::string_view _flagName);

	aura::AuraFlags parseAuraFlag(std::string_view _flagName);
	aura::EffectType parseAuraEffectType(std::string_view _effectName);
	aura::TriggerType parseAuraTriggerType(std::string_view _triggerName);

	StatModifierGroup parseEntityStatModifierGroupe(std::string_view _name);
	unit::StatType parseUnitStatType(std::string_view _effectName);
	unit::Condition parseUnitCondition(std::string_view _conditionName);
	std::string_view parseStatTypeToString(unit::StatType _type);
	unit::Modifier parseUnitModifier(std::string_view _name);
	std::string_view parseUnitModifierToString(unit::Modifier _mod);

	device::StatType parseDeviceStatType(std::string_view _effectName);
	std::string_view parseStatTypeToString(device::StatType _type);

	unit::MinionFlag parseMinionFlag(std::string_view _name);
	unit::TriggerFlag parseTriggerFlag(std::string_view _name);
	unit::ProjectileFlag parseProjectileFlag(std::string_view _name);
} // namespace game::data
