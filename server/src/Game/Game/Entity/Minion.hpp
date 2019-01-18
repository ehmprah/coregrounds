#pragma once

#include "Unit.hpp"
#include "Movable.hpp"
#include "Attackable.hpp"

namespace game::unit
{
	class PathMovementGenerator :
		public MovementGeneratorInterface
	{
	private:
		const map::RoutingMap& m_Map;
		Direction m_Direction = Direction::none;
		bool m_IgnoreCollision = false;
		const Minion& m_Owner;

		int _getPossibleDirections(const AbsPosition& _curPos) const;
		std::optional<std::pair<Direction, AbsPosition>> _calculateNextDirection(const AbsPosition& _pos) const;

	public:
		PathMovementGenerator(const Minion& _owner, const map::RoutingMap& _routingMap);

		std::optional<AbsPosition> getNext(const AbsPosition& _curPos) override;
		std::optional<AbsPosition> recalculateMovement(const AbsPosition& _curPos) override;
		bool ignoreCollision() const override;
	};

	struct MinionInitializer
	{
		UnitInitializer unitInit;
		const data::Minion& minionData;
		const TargetPreference* targetPreference;
	};

	class Minion :
		public Unit,
		private Movable,
		private Attackable
	{
		using super = Unit;

	public:
		Minion(const MinionInitializer& _initializer);

		virtual void applyUnitCondition(Condition _condition, bool _set = true) override;

		void setOwner(Participant& _owner) override;

		void update(Time _diff) override;

		const AbsShape& getCollider() const override;
		AbsPosition getPosition() const override;
		void setPosition(const AbsPosition& _pos) override;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;
		std::optional<AbsCoordType> getDistanceToDestination() const;
		void recalculateMovement();
		void setMovementGenerator(MovementGeneratorPtr _movementGenerator);
		unit::Type getType() const override;

		Stat getSpeed() const override;
		AbsCircle getAttackArea() const override;
		const TargetPreference* getTargetPreference() const override;

	private:
		const data::Minion& m_Data;
		UnitHandle m_LastCollidingUnit;

		// do not call this directly
		void _collisionListener(unit::Unit& _unit);

		void setupDerivedState(UnitState& _unitState) const override;
	};
} // namespace game::unit
