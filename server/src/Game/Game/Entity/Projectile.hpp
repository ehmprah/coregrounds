#pragma once

#include "Unit.hpp"
#include "Movable.hpp"

namespace game
{
namespace unit
{
	struct ProjectileInitializer
	{
		UnitInitializer unitInit;
		spell::Spell& spell;
		AbsPosition position;
		const data::Projectile& projectileData;
	};

	class Projectile :
		public Unit,
		private Movable
	{
		using super = Unit;

	public:
		Projectile(const ProjectileInitializer& _initializer);

		ProjectileHandle getHandle() const;

		void update(Time _diff) override;
		Type getType() const override;

		const AbsShape& getCollider() const override;
		Stat getSpeed() const override;
		AbsPosition getPosition() const override;
		void setPosition(const AbsPosition& _pos) override;
		AbsPosition getStartPosition() const;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;

		bool hasFinished() const;

	private:
		sl2::MasterHandle<unit::Projectile> m_MasterHandle;
		spell::Spell* m_Spell;
		std::vector<CastableHandle> m_OldTargets;
		const data::Projectile& m_Data;
		AbsPosition m_StartPosition;
		int m_Bounces = 0;
		AbsCircle m_BounceArea;

		bool _startChasing();
		// try to find an other target; returns true if successful
		bool _next();

		void setupDerivedState(UnitState& _unitState) const override;
	};
} // namespace unit
} // namespace game
