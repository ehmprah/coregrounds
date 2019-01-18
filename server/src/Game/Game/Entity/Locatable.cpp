#include "stdafx.h"
#include "Locatable.hpp"

namespace game::unit
{
	Locatable::Locatable(AbsShape _area)
	{
		setArea(std::move(_area));
	}

	const AbsShape& Locatable::getArea() const
	{
		return m_Area;
	}

	void Locatable::setArea(AbsShape _area)
	{
		m_Area = std::move(_area);
	}

	void Locatable::setPosition(const AbsPosition& _pos)
	{
		m_Area.setCenter(_pos);
	}

	AbsPosition Locatable::getPosition() const
	{
		return m_Area.getCenter();
	}

	AbsCoordType Locatable::getX() const
	{
		return getPosition().getX();
	}

	AbsCoordType Locatable::getY() const
	{
		return getPosition().getY();
	}

	std::optional<AbsPosition> Locatable::getDestination() const
	{
		return {};
	}

	DirectionVector Locatable::getDirectionVector() const
	{
		return DirectionVector(0, 0);
	}
} // namespace game::unit
