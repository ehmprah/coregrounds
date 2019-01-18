#pragma once

#include <nlohmann/json.hpp>
#include "StdInclude.hpp"

namespace json
{
	using Object = nlohmann::json;
	using Array = nlohmann::json::array_t;
	using Int = nlohmann::json::number_integer_t;
	using UInt = nlohmann::json::number_unsigned_t;
	using Float = nlohmann::json::number_float_t;

	Object& getElement(Object& _parent, std::string_view _name);
	const Object& getElement(const Object& _parent, std::string_view _name);

	Object* getElementPtr(Object& _parent, std::string_view _name);
	const Object* getElementPtr(const Object& _parent, std::string_view _name);

	template <class T>
	const T& get(const Object& _src, std::string_view _name)
	{
		return getElement(_src, _name).get_ref<const T&>();
	}

	template <class T>
	const T& get(const Object& _src, std::string_view _name, const T& _default)
	{
		try
		{
			if (auto elPtr = getElementPtr(_src, _name))
				return elPtr->get_ref<const T&>();
		}
		catch (...) {}
		return _default;
	}

	template <class T>
	const T* getPtr(const Object& _src, std::string_view _name)
	{
		try
		{
			return getElement(_src, _name).get_ptr<const T*>();
		}
		catch (...) {}
		return nullptr;
	}

	template <class T>
	T getValue(const Object& _src, std::string_view _name)
	{
		return getElement(_src, _name).get<T>();
	}

	template <class T>
	T getValue(const Object& _src, std::string_view _name, const T& _default)
	{
		try
		{
			if (auto elPtr = getElementPtr(_src, _name))
				return elPtr->get<T>();
		}
		catch (...) {}
		return _default;
	}

	template <class T>
	std::optional<T> getOptValue(const Object& _src)
	{
		try
		{
			return _src.get<T>();
		}
		catch (...) {}
		return std::nullopt;
	}

	const Array& getArray(const Object& _src, std::string_view _name);
	const Array* getArrayPtr(const Object& _src, std::string_view _name);
} // namespace json
