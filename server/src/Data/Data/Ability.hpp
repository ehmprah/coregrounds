#pragma once

#include "Device.hpp"

namespace game::data
{
	struct Ability : public Device
	{
	private:
		using super = Device;

	public:
		prop::Property<ID> spellId { prop::SpellId };
		prop::Property<std::chrono::milliseconds> cooldown { "cooldown", std::chrono::milliseconds(0), prop::DurationGreaterZero };
		using Flag = device::AbilityFlag;
		sl2::Bitmask<Flag> flags;

		void checkAndAdjust();
		std::string_view getTypeName() const override;
	};
} // namespace game::data
