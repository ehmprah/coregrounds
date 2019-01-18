#include "Json.hpp"

namespace json
{
	using lib = nlohmann::json;

	template <class TObj>
	TObj* _getElementPtr(TObj& _parent, std::string_view _name)
	{
		auto itr = _parent.find(_name);
		return itr != std::end(_parent) ? &*itr : nullptr;
	}

	template <class TObj>
	TObj& _getElement(TObj& _parent, std::string_view _name)
	{
		if (auto el = _getElementPtr(_parent, _name))
			return *el;
		throw std::runtime_error(std::string("json: no element with key: ") + _name.data());
	}

	json::Object& getElement(Object& _parent, std::string_view _name)
	{
		return _getElement(_parent, _name);
	}

	const json::Object& getElement(const Object& _parent, std::string_view _name)
	{
		return _getElement(_parent, _name);
	}

	json::Object* getElementPtr(Object& _parent, std::string_view _name)
	{
		return _getElementPtr(_parent, _name);
	}

	const json::Object* getElementPtr(const Object& _parent, std::string_view _name)
	{
		return _getElementPtr(_parent, _name);
	}

	const Array& getArray(const Object& _src, std::string_view _name)
	{
		return getElement(_src, _name).get_ref<const Array&>();
	}

	const json::Array* getArrayPtr(const Object& _src, std::string_view _name)
	{
		if (auto el = _getElementPtr(_src, _name))
			return el->get_ptr<const Array*>();
		return nullptr;
	}
} // namespace json
