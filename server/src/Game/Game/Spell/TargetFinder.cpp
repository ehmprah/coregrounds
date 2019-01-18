#include "stdafx.h"
#include "TargetFinder.hpp"
#include "Game/Entity/Unit.hpp"
#include "Game/Entity/Participant.hpp"
#include "Map/ColliderMap.hpp"
#include "Data/Factory.hpp"
#include "Shape.hpp"

namespace game::spell
{
	void _findFactories(const sl2::Bitmask<data::Spell::Target::Flags>& _mask, const device::Factories& _factories, std::vector<Castable*>& _targets)
	{
		using TargetFlags = data::Spell::Target::Flags;
		for (auto factory : _factories)
		{
			using DFac = game::data::Factory;
			auto& data = factory->getFactoryData();
			if (data.hasData<DFac::Minion>() && _mask.contains(TargetFlags::minion_factory) ||
				data.hasData<DFac::Tower>() && _mask.contains(TargetFlags::tower_factory) ||
				data.hasData<DFac::Wall>() && _mask.contains(TargetFlags::wall_factory))
				_targets.emplace_back(factory);
		}
	}

	void _findAbilities(const sl2::Bitmask<data::Spell::Target::Flags>& _mask, const device::Abilities& _abilities, std::vector<Castable*>& _targets)
	{
		using TargetFlags = data::Spell::Target::Flags;
		if (_mask.contains(TargetFlags::ability))
		{
			_targets.insert(std::end(_targets), std::begin(_abilities), std::end(_abilities));
		}
	}

	TargetFinder::TargetFinder(const Participant& _participant) :
		m_Participant(_participant)
	{
	}

	std::vector<TargetFinder::Unit*> TargetFinder::_getUniqueTargets(const TargetDefinition& _targetInfo, const AbsPosition& _pos, const AbsShape* _shape) const
	{
		auto& colliderMap = m_Participant.getGameDependencies().colliderMap;
		auto entities = _shape ? colliderMap.getUniqueFromCells(*_shape) : colliderMap.getCell(mapToTilePosition(_pos))->getEntities();
		std::vector<Unit*> units;
		units.reserve(entities.size());
		// remove all invalid objects
		for (auto entity : entities)
		{
			auto& owner = entity->getOwner();
			if (owner.isTargetable() && hasCorrectTargetFlags(_targetInfo, owner))
				units.emplace_back(&owner);
		}
		units.shrink_to_fit();
		return units;
	}


	void TargetFinder::_findUnits(const TargetDefinition& _targetInfo, const AbsPosition& _pos, std::vector<Unit*>& _targets, const AbsShape* _shape) const
	{
		using TargetFlags = data::Spell::Target::Flags;
		if ((*_targetInfo.friendlyMask & (uint32_t)TargetFlags::unit) == 0 &&
			(*_targetInfo.hostileMask & (uint32_t)TargetFlags::unit) == 0)
			return;

		auto units = _getUniqueTargets(_targetInfo, _pos, _shape);
		// check unit areas
		for (auto unit : units)
		{
			auto& oArea = unit->getCollider();
			if (_shape ? _shape->overlaps(oArea) : oArea.contains(_pos))
			{
				_targets.push_back(unit);
				if (_targetInfo.count > 0 && _targets.size() >= _targetInfo.count)
					break;
			}
		}
	}

	void TargetFinder::_findFactories(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const
	{
		spell::_findFactories(_targetInfo.friendlyMask, m_Participant.getFactories(), _targets);
		spell::_findFactories(_targetInfo.hostileMask, m_Participant.getOpponent().getFactories(), _targets);
	}

	void TargetFinder::_findAbilities(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const
	{
		spell::_findAbilities(_targetInfo.friendlyMask, m_Participant.getAbilities(), _targets);
		spell::_findAbilities(_targetInfo.hostileMask, m_Participant.getOpponent().getAbilities(), _targets);
	}

	std::vector<TargetFinder::Unit*> TargetFinder::findUnits(const TargetDefinition& _targetInfo, const AbsPosition& _pos, const data::Shape& _shape) const
	{
		AbsShape shape;
		auto posOnly = _shape.isNull();
		if (!posOnly)
		{
			shape = createShape(_shape);
			shape.setCenter(_pos);
		}

		std::vector<Unit*> targets;
		_findUnits(_targetInfo, _pos, targets, posOnly ? nullptr : &shape);
		return targets;
	}

	std::vector<TargetFinder::Unit*> TargetFinder::findUnits(const TargetDefinition& _targetInfo, const AbsShape& _shape) const
	{
		std::vector<Unit*> targets;
		_findUnits(_targetInfo, _shape.getCenter(), targets, &_shape);
		return targets;
	}

	std::vector<TargetFinder::Castable*> TargetFinder::findDevices(const TargetDefinition& _targetInfo) const
	{
		std::vector<Castable*> targets;
		_findFactories(_targetInfo, targets);
		_findAbilities(_targetInfo, targets);
		return targets;
	}

	bool TargetFinder::hasCorrectTargetFlags(const data::Spell::Target& _targetInfo, const Castable& _target) const
	{
		using TargetFlag = data::Spell::Target::Flags;
		using UnitType = unit::Type;
		if (_target.isUnit())
		{
			auto& unit = _target.toUnit();
			switch (unit.getType())
			{
			case UnitType::core:
				return unit::isFriendly(unit, m_Participant) ? _targetInfo.friendlyMask.contains(TargetFlag::core) :
					_targetInfo.hostileMask.contains(TargetFlag::core);
			case UnitType::wall:
				return unit::isFriendly(unit, m_Participant) ? _targetInfo.friendlyMask.contains(TargetFlag::wall) :
					_targetInfo.hostileMask.contains(TargetFlag::wall);
			case UnitType::tower:
				return unit::isFriendly(unit, m_Participant) ? _targetInfo.friendlyMask.contains(TargetFlag::tower) :
					_targetInfo.hostileMask.contains(TargetFlag::tower);
			case UnitType::minion:
				return unit::isFriendly(unit, m_Participant) ? _targetInfo.friendlyMask.contains(TargetFlag::minion) :
					_targetInfo.hostileMask.contains(TargetFlag::minion);
			}
			return false;
		}
		else if (_target.isDevice())
		{
			return true;
		}
		return false;
	}
} // namespace game::spell
