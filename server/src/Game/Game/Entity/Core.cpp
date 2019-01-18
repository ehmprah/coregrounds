#include "stdafx.h"
#include "Core.hpp"
#include "Data/Game.hpp"
#include "Data/Unit.hpp"
#include "Core/HelperFunctions.hpp"
#include "Shape.hpp"

namespace game::unit
{
	Core::Core(const CoreInitializer& _initializer) :
		Locatable(createShape(_initializer.coreData.getBody())),
		super(_initializer.unitInit),
		m_Data(_initializer.coreData)
	{
		setUnitFlag(unit::Flags::unsolid, true);
		setPosition(mapToAbsPosition(_initializer.pos + TileAbsoluteSize / 2)); // setup at tile center
	}

	void Core::setOwner(Participant& _owner)
	{
		LOG_ERR("Core: setOwner: not allowed for Core types.");
	}

	const AbsShape& Core::getCollider() const
	{
		return Locatable::getArea();
	}

	AbsPosition Core::getPosition() const
	{
		return Locatable::getPosition();
	}

	void Core::setPosition(const AbsPosition& _pos)
	{
		auto newPos = mapToAbsPosition(mapToTilePosition(_pos)) + TileAbsoluteSize / 2;
		if (newPos != getPosition())
		{
			Locatable::setPosition(newPos);
			super::adjustCollider();
		}
	}

	std::optional<game::AbsPosition> Core::getDestination() const
	{
		return Locatable::getDestination();
	}

	DirectionVector Core::getDirectionVector() const
	{
		return Locatable::getDirectionVector();
	}

	unit::Type Core::getType() const
	{
		return unit::Type::core;
	}

	bool Core::isErasable() const
	{
		return false;
	}

	void Core::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = CoreState{ std::min(getHealthPercent(_unitState), getHealthPercent(getUnitState())) };
	}
} // namespace game::unit
