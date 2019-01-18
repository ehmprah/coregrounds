#pragma once

namespace game
{
	bool isPreferedType(const TargetPreference& _targetPref, const unit::Unit& _unit);
	bool fulfillsFlags(const TargetPreference& _targetPref, const unit::Unit& _unit);
	std::vector<unit::Unit*> filterUnits(const TargetPreference& _targetPref, const std::vector<unit::Unit*>& _units);
	unit::Unit* selectUnit(const TargetPreference& _targetPref, const std::vector<unit::Unit*>& _units, const Participant& _owner);
} // namespace game
