#pragma once

#include "Movable.hpp"

namespace game::unit
{
	class BaseCollisionDetector :
		public AbstractCollisionDetector
	{
	public:
		BaseCollisionDetector(const map::ColliderMap& _colliderMap, const Unit& _this);

		// returns unique objects from cells (without self)
		std::vector<Unit*> getIntersectingUnits(const AbsLine& _line) const;

		std::vector<CollisionInfo> checkMovement(const AbsPosition& _movement) const override;

	private:
		using super = AbstractCollisionDetector;

		const map::ColliderMap& m_ColliderMap;
	};
} // namespace game::unit
