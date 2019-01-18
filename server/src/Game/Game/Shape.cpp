#include "stdafx.h"
#include "Shape.hpp"

namespace game
{
	AbsShape createShape(const data::Shape& _data)
	{
		if (_data.hasData<data::Shape::Rect>())
		{
			auto& data = _data.getData<data::Shape::Rect>();
			return AbsShape(AbsRect(*_data.x, *_data.y, *data.width, *data.height));
		}
		if (_data.hasData<data::Shape::Circle>())
		{
			auto& data = _data.getData<data::Shape::Circle>();
			return AbsShape(AbsCircle(*_data.x, *_data.y, *data.radius));
		}
		throw std::runtime_error("CreateShape: invalid data.");
	}
} // namespace game
