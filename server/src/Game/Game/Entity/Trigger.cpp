#include "stdafx.h"
#include "Trigger.hpp"
#include "Data/Unit.hpp"
#include "Map/VisibilityMap.hpp"
#include "Participant.hpp"
#include "Shape.hpp"

namespace game::unit
{
	Trigger::Trigger(const TriggerInitializer& _initializer) :
		Locatable(createShape(_initializer.triggerData.getBody())),
		super(_initializer.unitInit),
		triggerData(_initializer.triggerData)
	{
		assert(triggerData.lifeTime->count() > 0);
		m_LifeTimer.start(*triggerData.lifeTime);
		setUnitFlag(unit::Flags::unsolid);
		setUnitFlag(unit::Flags::noVisibilityModification, !triggerData.getFlags().contains(TriggerFlag::modifyVisibility));
	}

	void Trigger::update(Time _diff)
	{
		super::update(_diff);

		m_LifeTimer.update(_diff);
		if (m_LifeTimer.getState() == Timer::State::finished)
			kill();
	}

	Type Trigger::getType() const
	{
		return Type::trigger;
	}

	const AbsShape& Trigger::getCollider() const
	{
		return Locatable::getArea();
	}

	void Trigger::setPosition(const AbsPosition& _pos)
	{
		if (_pos != getPosition())
		{
			Locatable::setPosition(_pos);
		}
	}

	AbsPosition Trigger::getPosition() const
	{
		return Locatable::getPosition();
	}

	std::optional<game::AbsPosition> Trigger::getDestination() const
	{
		return Locatable::getDestination();
	}

	DirectionVector Trigger::getDirectionVector() const
	{
		return Locatable::getDirectionVector();
	}

	void Trigger::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = TowerState();
	}
} // namespace game::unit
