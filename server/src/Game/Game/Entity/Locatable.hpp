#pragma once

namespace game::unit
{
	/*\brief The locatable component for entities.
	*
	* This inheritable component provides basic position functionality for the entity system.
	* There is no update call needed.*/
	class Locatable
	{
	private:
		AbsShape m_Area;

	public:
		Locatable(AbsShape _area);
		virtual ~Locatable() = default;

		/*\brief Returns the current shape
		*\return Const reference to the current area*/
		const AbsShape& getArea() const;

		/*\brief Change current area
		*\param _area new area.*/
		void setArea(AbsShape _area);

		AbsPosition getPosition() const;
		AbsCoordType getX() const;
		AbsCoordType getY() const;
		virtual void setPosition(const AbsPosition& _pos);

		virtual std::optional<AbsPosition> getDestination() const;
		virtual DirectionVector getDirectionVector() const;
	};
} // namespace game::unit
