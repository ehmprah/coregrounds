#include "Shape.hpp"
#include "Core/log.hpp"
#include "Others.hpp"

namespace game::data
{
	//Shape _mod(Shape _lhs, const Shape& _rhs)
	//{
	//	if (_lhs.typeID == _rhs.typeID)
	//	{
	//		_lhs.center += _rhs.center;
	//		switch (_lhs.typeID)
	//		{
	//		case game::area::Type::none:
	//		case game::area::Type::max:
	//			break;
	//		case game::area::Type::rectangle:
	//			_lhs.area.rectangle.width = std::max<AbsCoordType>(0, _lhs.area.rectangle.width + _rhs.area.rectangle.width);
	//			_lhs.area.rectangle.height = std::max<AbsCoordType>(0, _lhs.area.rectangle.height + _rhs.area.rectangle.height);
	//			break;
	//		case game::area::Type::circle:
	//			_lhs.area.circle.radius = std::max<AbsCoordType>(0, _lhs.area.circle.radius + _rhs.area.circle.radius);
	//			break;
	//		}
	//	}
	//	else
	//		LOG_ERR("Shape: modify: Types are different.");
	//	return _lhs;
	//}

	bool Shape::isNull() const
	{
		return std::holds_alternative<std::monostate>(m_Data);
	}

	Shape Shape::modify(const AreaModifier& _mod) const
	{
		switch (_mod.type)
		{
		case AreaModifier::Type::set: return _mod.data;
		case AreaModifier::Type::mod: throw Error("Shape: mod not implemented");
		}
		throw Error("Shape: invalid mod type.");
	}
} // namespace game::data
