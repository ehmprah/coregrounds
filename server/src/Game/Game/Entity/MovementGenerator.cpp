#include "stdafx.h"
#include "MovementGenerator.hpp"
#include "Unit.hpp"

namespace game::unit
{
	/*#####
	# ProjectilePositionMovementGenerator
	#####*/
	ProjectilePositionMovementGenerator::ProjectilePositionMovementGenerator(const AbsPosition& _position) :
		m_Position(_position)
	{
	}

	std::optional<AbsPosition> ProjectilePositionMovementGenerator::getNext(const AbsPosition& _curPos)
	{
		if (!isAlmostEqual(_curPos, m_Position))
			return m_Position;
		return std::nullopt;
	}

	std::optional<AbsPosition> ProjectilePositionMovementGenerator::recalculateMovement(const AbsPosition& _curPos)
	{
		return getNext(_curPos);
	}

	bool ProjectilePositionMovementGenerator::ignoreCollision() const
	{
		return true;
	}

	/*#####
	# ProjectileTargetMovementGenerator
	#####*/
	ProjectileTargetMovementGenerator::ProjectileTargetMovementGenerator(const Unit& _target) :
		m_Target(_target)
	{
	}

	std::optional<AbsPosition> ProjectileTargetMovementGenerator::getNext(const AbsPosition& _curPos)
	{
		auto targetPos = m_Target.getPosition();
		if (!isAlmostEqual(_curPos, targetPos))
			return targetPos;
		return std::nullopt;
	}

	std::optional<AbsPosition> ProjectileTargetMovementGenerator::recalculateMovement(const AbsPosition& _curPos)
	{
		return getNext(_curPos);
	}

	bool ProjectileTargetMovementGenerator::ignoreCollision() const
	{
		return true;
	}
} // namespace game::unit
