#pragma once

#include "Movable.hpp"

namespace game::unit
{
	class ProjectilePositionMovementGenerator :
		public MovementGeneratorInterface
	{
	private:
		const AbsPosition& m_Position;

	public:
		ProjectilePositionMovementGenerator(const AbsPosition& _position);

		std::optional<AbsPosition> getNext(const AbsPosition& _curPos) override;
		std::optional<AbsPosition> recalculateMovement(const AbsPosition& _curPos) override;
		bool ignoreCollision() const override;
	};

	class ProjectileTargetMovementGenerator :
		public MovementGeneratorInterface
	{
	private:
		const Unit& m_Target;

	public:
		ProjectileTargetMovementGenerator(const Unit& _target);

		std::optional<AbsPosition> getNext(const AbsPosition& _curPos) override;
		std::optional<AbsPosition> recalculateMovement(const AbsPosition& _curPos) override;
		bool ignoreCollision() const override;
	};
} // namespace game::unit
