#include "stdafx.h"
#include "Movable.hpp"
#include "Unit.hpp"

namespace game::unit
{
	/*#####
	# AbstractCollisionDetector
	#####*/
	AbstractCollisionDetector::AbstractCollisionDetector(const Unit& _this) :
		m_This(_this)
	{
	}

	const Unit& AbstractCollisionDetector::getThis() const
	{
		return m_This;
	}

	CollisionInfo::CollisionInfo(AbsPosition _collision, Unit& _collidingUnit) :
		distance(_collision),
		collidingUnit(&_collidingUnit)
	{
	}

	/*#####
	# Movable
	#####*/
	Movable::Movable(AbsShape _shape) :
		Locatable(std::move(_shape))
	{
	}

	std::pair<AbsCoordType, bool> Movable::_updateMovement(Stat _speed)
	{
		assert(m_MovementGenerator);
		if (auto destination = getDestination())
		{
			auto pos = getPosition();
			auto distance = sl2::calculateDistance(pos, *destination);
			AbsPosition moveVector(m_DirectionVector * (distance < _speed ? distance : _speed));

			bool colliding = false;
			if (m_CollisionDetector && !m_MovementGenerator->ignoreCollision())
			{
				if (auto collisionInfos = m_CollisionDetector->checkMovement(moveVector); !collisionInfos.empty())
				{
					moveVector = collisionInfos.front().distance;
					colliding = true;
				}
			}

			pos += moveVector;

			auto move = sl2::calculateLength(moveVector);
			assert(move >= 0);
			setPosition(pos);
			return{ move, colliding };
		}
		return{ _speed, false };
	}

	void Movable::_setDestination(std::optional<AbsPosition> _pos)
	{
		if (_pos)
		{
			auto diff = *_pos - getPosition();
			if (std::abs(diff.getX()) <= std::numeric_limits<AbsCoordType>::epsilon())
				diff.setX(0);
			if (std::abs(diff.getY()) <= std::numeric_limits<AbsCoordType>::epsilon())
				diff.setY(0);
			m_DirectionVector = sl2::createNormalized(diff);
		}
		m_Destination = std::move(_pos);
	}

	std::optional<AbsPosition> Movable::getDestination() const
	{
		return m_Destination;
	}

	DirectionVector Movable::getDirectionVector() const
	{
		return m_DirectionVector;
	}

	std::optional<AbsCoordType> Movable::getDistanceToDestination() const
	{
		if (auto destination = getDestination())
			return sl2::calculateDistance(getPosition(), *destination);
		return std::nullopt;
	}

	void Movable::updateMovement(Time _diff)
	{
		assert(m_MovementGenerator);
		auto speed = getSpeed() * _diff.count() / 1000;
		while (Epsilon < speed && getDestination())
		{
			auto lastSpeed = speed;
			auto result = _updateMovement(speed);
			speed -= result.first;
			if (reachedDestination())
				_setDestination(m_MovementGenerator->getNext(getPosition()));
			else if (result.second || lastSpeed > speed + Epsilon)
				break;
		}
	}

	void Movable::startMovement()
	{
		if (!getDestination())
		{
			assert(m_MovementGenerator);
			_setDestination(m_MovementGenerator->getNext(getPosition()));
		}
	}

	void Movable::recalculateMovement()
	{
		assert(m_MovementGenerator);
		_setDestination(m_MovementGenerator->recalculateMovement(getPosition()));
	}

	void Movable::setMovementGenerator(MovementGeneratorPtr _movementGenerator)
	{
		m_MovementGenerator = std::move(_movementGenerator);
		assert(m_MovementGenerator);
	}

	void Movable::setCollisionDetector(CollisionDetectorPtr _collisionDetector)
	{
		m_CollisionDetector = std::move(_collisionDetector);
	}

	bool Movable::reachedDestination() const
	{
		auto dest = getDestination();
		return dest && isAlmostEqual(getPosition(), *dest);
	}
} // namespace game::unit
