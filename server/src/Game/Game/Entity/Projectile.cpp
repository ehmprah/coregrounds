#include "stdafx.h"
#include "Projectile.hpp"
#include "Data/Unit.hpp"
#include "MovementGenerator.hpp"
#include "Participant.hpp"
#include "Shape.hpp"

namespace game::unit
{
	/*#####
	# Projectile
	#####*/
	Projectile::Projectile(const ProjectileInitializer& _initializer) :
		Movable(createShape(_initializer.projectileData.getBody())),
		super(_initializer.unitInit),
		m_Data(_initializer.projectileData),
		m_Spell(&_initializer.spell),
		m_BounceArea(AbsCircle(0, 0, *_initializer.projectileData.bounceRange)),
		m_MasterHandle(*this)
	{
		setPosition(_initializer.position);
		_startChasing();
	}

	ProjectileHandle Projectile::getHandle() const
	{
		return m_MasterHandle.get();
	}

	bool Projectile::_startChasing()
	{
		assert(m_Spell);
		auto entityTarget = m_Spell->getDestinationEntity();
		if (entityTarget && entityTarget->isUnit())
		{
			auto& unitTarget = entityTarget->toUnit();
			m_OldTargets.push_back(unitTarget.getCastableHandle());
			setMovementGenerator(std::make_unique<unit::ProjectileTargetMovementGenerator>(unitTarget));
		}
		else
		{
			if (auto pos = m_Spell->getDestinationPosition())
				setMovementGenerator(std::make_unique<unit::ProjectilePositionMovementGenerator>(*pos));
			else
				return false;
		}
		m_StartPosition = getPosition();
		startMovement();
		return true;
	}

	bool Projectile::_next()
	{
		assert(m_Spell);
		m_BounceArea.setCenter(getPosition());
		if (m_Bounces >= m_Data.bounces || !m_Spell->changeDestination(m_BounceArea, m_Data.getFlags().contains(ProjectileFlag::uniqueHit)) || !_startChasing())
			return false;
		++m_Bounces;
		return true;
	}

	void Projectile::update(Time _diff)
	{
		super::update(_diff);
		if (hasFinished() || unit::isDead(*this))
			return;

		if (!hasCondition(*this, Condition::root))
		{
			// We recalculate movement here, to react on possible target movement
			recalculateMovement();
			auto destination = getDestination();
			Movable::updateMovement(_diff);
			if (!destination || getArea().contains(*destination))
			{
				m_Spell->exec();
				if (!_next())
					m_Spell = nullptr;
			}
		}
	}

	Type Projectile::getType() const
	{
		return Type::projectile;
	}

	Stat Projectile::getSpeed() const
	{
		return getUnitState().stats.getStat(StatType::moveSpeed);
	}

	bool Projectile::hasFinished() const
	{
		return m_Spell == nullptr;
	}

	const AbsShape& Projectile::getCollider() const
	{
		return Movable::getArea();
	}

	AbsPosition Projectile::getPosition() const
	{
		return Movable::getPosition();
	}

	void Projectile::setPosition(const AbsPosition& _pos)
	{
		if (_pos != getPosition())
			Locatable::setPosition(_pos);
	}

	AbsPosition Projectile::getStartPosition() const
	{
		return m_StartPosition;
	}

	std::optional<game::AbsPosition> Projectile::getDestination() const
	{
		return Movable::getDestination();
	}

	DirectionVector Projectile::getDirectionVector() const
	{
		return Movable::getDirectionVector();
	}

	void Projectile::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = ProjectileState{ getDirectionVector() };
	}
} // namespace game::unit
