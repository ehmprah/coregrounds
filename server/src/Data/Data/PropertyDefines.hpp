#pragma once

#include "Property.hpp"
#include "Core/SharedDefines.hpp"

namespace game::prop
{
	const auto IdValidator = [](ID _id) -> bool {
		if (0 < _id)
			return true;
		throw std::runtime_error("Invalid id: " + std::to_string(_id));
	};

	const auto GreaterZero = [](auto&& _value) {
		return 0 < _value;
	};

	const auto GreaterEqualZero = [](auto&& _value) {
		return 0 <= _value;
	};

	const auto LessZero = [](auto&& _value) {
		return _value < 0;
	};

	const auto LessEqualZero = [](auto&& _value) {
		return _value <= 0;
	};

	const auto DurationGreaterZero = [](auto&& _value) {
		return 0 < _value.count();
	};

	const auto DurationGreaterEqualZero = [](auto&& _value) {
		return 0 <= _value.count();
	};

	const Property<ID> Id{ "id", 0, IdValidator };
	const Property<ID> UnlockId{ "unlockId", 0, GreaterEqualZero };
	const Property<ID> SpellId{ "spellId", 0, GreaterEqualZero };
	const Property<std::string> Name{ "name", "" };
} // namespace game::prop
