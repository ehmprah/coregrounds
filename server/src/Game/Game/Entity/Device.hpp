#pragma once

#include "Entity.hpp"

namespace game::device
{
	struct DeviceInitializer
	{
		EntityInitializer entityInit;
		const data::Device& deviceData;
	};

	class Device :
		public Entity
	{
		using super = Entity;

	public:
		using StatModifier = StatModifier<StatType>;
		StatModifier statModifier;
		const data::Device& deviceData;

		Device(const DeviceInitializer& _initializer);

		virtual Stat calculateStat(StatType _type) const;
		Stat getStatBase(StatType _type) const;

		bool isParticipant() const final;

		bool isUnit() const final;
		const unit::Unit& toUnit() const final;
		unit::Unit& toUnit() final;

		bool isDevice() const final;
		const device::Device& toDevice() const final;
		device::Device& toDevice() final;
	};

	Stat getStat(const Device& _device, StatType _type);
} // namespace game::device

namespace game
{
	Time updateTimerRelatedToStat(Timer& _timer, Time _diff, Time _baseDuration, Stat _stat);
} // namespace game
