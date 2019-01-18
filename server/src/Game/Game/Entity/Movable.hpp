#pragma once

#include "Locatable.hpp"

namespace game::unit
{
	class MovementGeneratorInterface
	{
	public:
		virtual ~MovementGeneratorInterface() = default;

		virtual std::optional<AbsPosition> getNext(const AbsPosition& _curPos) = 0;
		virtual std::optional<AbsPosition> recalculateMovement(const AbsPosition& _curPos) = 0;
		virtual bool ignoreCollision() const = 0;
	};
	using MovementGeneratorPtr = std::unique_ptr<MovementGeneratorInterface>;

	struct CollisionInfo
	{
		AbsPosition distance;
		Unit* collidingUnit;

		CollisionInfo(AbsPosition _range, Unit& _collidingUnit);
	};
	class AbstractCollisionDetector :
		sl2::NonCopyable
	{
	private:
		const Unit& m_This;

	public:
		AbstractCollisionDetector(const Unit& _this);
		virtual ~AbstractCollisionDetector() = default;

		const Unit& getThis() const;

		virtual std::vector<CollisionInfo> checkMovement(const AbsPosition& _movement) const = 0;
	};
	using CollisionDetectorPtr = std::unique_ptr<AbstractCollisionDetector>;

	class Movable :
		public Locatable
	{
	private:
		std::optional<AbsPosition> m_Destination;
		DirectionVector m_DirectionVector;
		MovementGeneratorPtr m_MovementGenerator;
		CollisionDetectorPtr m_CollisionDetector;

		std::pair<AbsCoordType/*range without colliding*/, bool/*colliding*/> _updateMovement(Stat _speed);
		void _setDestination(std::optional<AbsPosition> _pos);

	protected:
		void updateMovement(Time _diff);

	public:
		Movable(AbsShape _shape);
		virtual ~Movable() = default;

		virtual Stat getSpeed() const = 0;

		void startMovement();
		void recalculateMovement();
		void setMovementGenerator(MovementGeneratorPtr _movementGenerator);
		void setCollisionDetector(CollisionDetectorPtr _collisionDetector);
		bool reachedDestination() const;

		std::optional<AbsCoordType> getDistanceToDestination() const;

		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;
	};
} // namespace game::unit
