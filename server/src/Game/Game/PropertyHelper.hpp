#pragma once

#include "Data/Property.hpp"

namespace game::prop
{
namespace
{
	template <class T>
	std::optional<T> _assign(std::string_view _name, const json::Object& _obj)
	{
		try
		{
			if constexpr (std::is_same_v<T, Time>)	// we expect input for Time as double value with 1 == 1sec
			{
				auto value = json::getValue<double>(_obj, _name);
				return Time(static_cast<int>(value * 1000));
			}
			else
			{
				return json::getValue<T>(_obj, _name);
			}
		}
		catch (...) {}
		return std::nullopt;
	}
} // anonymous namespace

	template <class T>
	Property<T>& assign(Property<T>& _property, const json::Object& _obj)
	{
		if (auto value = _assign<T>(_property.getName(), _obj))
			_property = std::move(*value);
		return _property;
	}

	template <class T>
	Property<std::optional<T>>& assign(Property<std::optional<T>>& _property, const json::Object& _obj)
	{
		_property.setValue(_assign<T>(_property.getName(), _obj));
		return _property;
	}
} // namespace game::prop
