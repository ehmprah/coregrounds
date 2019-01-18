#include "stdafx.h"
#include "CollisionDetector.hpp"
#include "Unit.hpp"
#include "Data/Unit.hpp"
#include "Map/ColliderMap.hpp"

namespace game::unit
{
	BaseCollisionDetector::BaseCollisionDetector(const map::ColliderMap& _colliderMap, const Unit& _this) :
		super(_this),
		m_ColliderMap(_colliderMap)
	{
	}

	bool _ignoreCollision(const Unit& _lhs, const Unit& _rhs)
	{
		if (*_lhs.unitData.layer != *_rhs.unitData.layer ||
			unit::isHostile(_lhs, _rhs) && (*_lhs.unitData.layer != 0 || *_rhs.unitData.layer != 0))
			return true;

		if (unit::isHostile(_lhs, _rhs) || _lhs.getDirectionVector() == _rhs.getDirectionVector())
			return false;

		auto lhsDestination = _lhs.getDestination();
		auto rhsDestination = _rhs.getDestination();
		// ToDo: possible better result with getNearestPoint for the colliders?
		if (lhsDestination && lhsDestination == rhsDestination)
		{
			return sl2::calculateDistanceSquared(*lhsDestination, _lhs.getCollider().getCenter()) <=
				sl2::calculateDistanceSquared(*rhsDestination, _rhs.getCollider().getCenter());
		}
		return false;
	}

	std::vector<Unit*> BaseCollisionDetector::getIntersectingUnits(const AbsLine& _line) const
	{
		auto center = getThis().getPosition();
		std::vector<Unit*> units;
		for (auto cell : m_ColliderMap.calculateCells(_line))
		{
			auto& entities = cell->getEntities();
			units.reserve(units.size() + entities.size());
			for (auto entity : entities)
			{
				auto& unit = entity->getOwner();
				if (&unit != &getThis() &&
					!unit.hasUnitFlag(unit::Flags::unsolid) &&
					std::find(std::begin(units), std::end(units), &unit) == std::end(units) &&
					(unit.getCollider().intersects(_line) || unit.getCollider().contains(_line.getPosition())) &&
					!_ignoreCollision(getThis(), unit))
					units.push_back(&unit);
			}
		}
		units.shrink_to_fit();
		return units;
	}

	Unit* _getClosestUnit(const std::vector<Unit*>& _units, const AbsPosition& _center)
	{
		assert(!_units.empty());

		if (_units.size() == 1)
			return _units.front();

		// check for shortest distance
		Unit* curUnit = nullptr;
		auto shortestDistance = std::numeric_limits<AbsCoordType>::max();
		for (auto unit : _units)
		{
			auto dist = sl2::calculateDistanceSquared(unit->getCollider().getCenter(), _center);
			if (dist < shortestDistance)
			{
				if (dist <= 1)
					return unit;
				curUnit = unit;
				shortestDistance = dist;
			}
		}
		return curUnit;
	}

	std::vector<CollisionInfo> BaseCollisionDetector::checkMovement(const AbsPosition& _movement) const
	{
		// This functions handles only axis aligned movement. So do NOT pass a diagonal vector
		// ToDo: fix this behavior
		if (_movement.getX() != 0 && _movement.getY() != 0)
		{
			LOG_ERR("Unit movement is not axis aligned. Destination: " << getThis().getDestination() << " Movement-Vector: " << _movement);
			return {};
		}

		auto length = sl2::calculateLengthSquared(_movement);
		if (length == 0)
			return {};

		auto normalized = sl2::createNormalized(_movement);
		AbsLine line(getThis().getPosition(), sl2::geometry2d::Slope<AbsCoordType>(_movement + normalized), sl2::geometry2d::LineType::segment);
		auto units = getIntersectingUnits(line);
		std::vector<CollisionInfo> infos;

		// we use this line to get a 90 degree border from our movement direction, to determine if an other units center is "in front" of this or not.
		AbsLine borderLine(getThis().getPosition(), sl2::geometry2d::Slope<AbsCoordType>({ -_movement.getY(), _movement.getX() }), sl2::geometry2d::LineType::line);

		auto normMovement = sl2::createNormalized(_movement);
		for (auto unit : units)
		{
			assert(unit);
			if (!sl2::geometry2d::isRight(borderLine, unit->getPosition()))
			{
				if (getThis().getCollider().overlaps(unit->getCollider()))
				{
					if (getThis().getPosition() != unit->getPosition() ||
						unit->getGUID() < getThis().getGUID())
					{
						infos.emplace(std::begin(infos), AbsPosition(), *unit);
					}
				}
				else if (auto touchingDistance = getThis().getCollider().calculateTouchingDistance(unit->getCollider(), _movement);
					touchingDistance && *touchingDistance * *touchingDistance <= length)
				{
					CollisionInfo info{ normMovement * *touchingDistance, *unit };
					auto cachedDistance = sl2::calculateDistanceSquared(AbsPosition(), info.distance);
					infos.emplace(std::lower_bound(std::begin(infos), std::end(infos), info, [cachedDistance](const auto& _lhs, const auto& _rhs) {
						return sl2::calculateDistanceSquared(AbsPosition(), _lhs.distance) < cachedDistance;
					}), info);
				}
			}
		}
		return infos;
	}
} // namespace game::unit
