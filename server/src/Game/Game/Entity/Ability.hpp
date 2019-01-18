#pragma once

#include "Device.hpp"

namespace game::device
{
	struct AbilityInitializer
	{
		::game::device::DeviceInitializer deviceInit;
		const data::Ability& abilityData;
	};

	class Ability :
		public ::game::device::Device
	{
	public:
		Ability(const AbilityInitializer& _initializer);

		const data::Ability& getAbilityData() const;
		int getUses() const;
		void activate(const AbsPosition& _pos);

		void update(Time _diff) override;
		void serialize(protobuf::out::Match_Game_Ability& _msg) const;
	
	protected:
		void derivedStartPlay() override;

	private:
		using super = ::game::device::Device;

		const data::Ability& m_AbilityData;
		const data::Spell& m_SpellData;
		Timer m_Cooldown;

		int m_Uses = 0;

		bool _cast(const AbsPosition& _pos);
	};
	using AbilityPtr = std::unique_ptr<Ability>;
} // namespace game::device
