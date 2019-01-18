#include "stdafx.h"
#include "TargetPreferenceUtil.hpp"
#include "Game/Entity/Core.hpp"
#include "Game/Entity/Participant.hpp"

namespace game
{
	bool isPreferedType(const TargetPreference& _targetPref, const unit::Unit& _unit)
	{
		using UnitType = unit::Type;
		using Target = unit::targetPreference::Target;
		switch (_unit.getType())
		{
		case UnitType::core: return _targetPref.target.contains(Target::core);
		case UnitType::wall: return _targetPref.target.contains(Target::wall);
		case UnitType::tower: return _targetPref.target.contains(Target::tower);
		case UnitType::minion: return _targetPref.target.contains(Target::minion);
		}
		return false;
	}

	bool fulfillsFlags(const TargetPreference& _targetPref, const unit::Unit& _unit)
	{
		using Flag = unit::targetPreference::Flag;
		if (_targetPref.flags.contains(Flag::damaged) &&
			_unit.getUnitState().stats.getStat(unit::StatType::maxHealth) <= _unit.getUnitState().health)
			return false;
		return true;
	}

	std::vector<unit::Unit*> filterUnits(const TargetPreference& _targetPref, const std::vector<unit::Unit*>& _units)
	{
		std::vector<unit::Unit*> result;
		result.reserve(_units.size());
		std::copy_if(std::begin(_units), std::end(_units), std::back_inserter(result),
			[&_targetPref](auto _unit)
			{
				assert(_unit);
				return isPreferedType(_targetPref, *_unit) && fulfillsFlags(_targetPref, *_unit);
			}
		);
		return result;
	}

	unit::Unit* _selectUnitByMode(const std::vector<unit::Unit*>& _units, TargetPreference::Mode _mode, const Participant& _owner)
	{
		if (!_units.empty())
		{
			auto orderByType = [](unit::Unit* _lhs, unit::Unit* _rhs)
			{
				using UnitType = unit::Type;
				auto typeToValue = [](UnitType _type)
				{
					switch (_type)
					{
					case UnitType::core: return 0;
					case UnitType::tower: return 1;
					case UnitType::minion: return 2;
					}
					return 3;
				};
				assert(_lhs && _rhs);
				auto lhsVal = typeToValue(_lhs->getType());
				auto rhsVal = typeToValue(_rhs->getType());
				return lhsVal < rhsVal;
			};

			switch (_mode)
			{
			case TargetPreference::Mode::high:
				return *std::max_element(std::begin(_units), std::end(_units),
					[orderByType](auto _lhs, auto _rhs)
					{
						return orderByType(_rhs, _lhs) ||
							!orderByType(_lhs, _rhs) && _lhs->getUnitState().health < _rhs->getUnitState().health;
					}
				);
			case TargetPreference::Mode::low:
				return *std::min_element(std::begin(_units), std::end(_units),
					[orderByType](auto _lhs, auto _rhs)
					{
						return orderByType(_lhs, _rhs) ||
							!orderByType(_rhs, _lhs) && _lhs->getUnitState().health < _rhs->getUnitState().health;
					}
				);
			case TargetPreference::Mode::first:
			{
				unit::Unit* result = nullptr;
				AbsCoordType minDistance = std::numeric_limits<AbsCoordType>::max();
				auto corePos = _owner.getCore().getPosition();
				for (auto unit : _units)
				{
					auto distance = sl2::calculateDistanceSquared(corePos, unit->getPosition());
					if (distance < minDistance)
					{
						result = unit;
						minDistance = distance;
					}
				}
				assert(result);
				return result;
			}
			case TargetPreference::Mode::random: return _units[randomInt<std::size_t>(0, _units.size() - 1)];
			}
		}
		return nullptr;
	}

	unit::Unit* selectUnit(const TargetPreference& _targetPref, const std::vector<unit::Unit*>& _units, const Participant& _owner)
	{
		return _selectUnitByMode(filterUnits(_targetPref, _units), _targetPref.mode, _owner);
	}
} // namespace game
