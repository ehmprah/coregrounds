#include "stdafx.h"
#include "Device.hpp"
#include "Data/Device.hpp"

namespace game::device
{
	Device::Device(const DeviceInitializer& _initializer) :
		super(_initializer.entityInit),
		deviceData(_initializer.deviceData)
	{
	}

	Stat Device::calculateStat(StatType _type) const
	{
		return statModifier.modifyValue(_type, getStatBase(_type));
	}

	Stat Device::getStatBase(StatType _type) const
	{
		return deviceData.getStats().getStat(_type);
	}

	bool Device::isParticipant() const
	{
		return false;
	}

	bool Device::isUnit() const
	{
		return false;
	}

	const unit::Unit& Device::toUnit() const
	{
		throw std::runtime_error("Castable: Is no Unit type.");
	}

	unit::Unit& Device::toUnit()
	{
		throw std::runtime_error("Castable: Is no Unit type.");
	}

	bool Device::isDevice() const
	{
		return true;
	}

	const device::Device& Device::toDevice() const
	{
		return *this;
	}

	device::Device& Device::toDevice()
	{
		return *this;
	}

	Stat getStat(const Device& _device, StatType _type)
	{
		return _device.calculateStat(_type);
	}
} // namespace game::device

namespace game
{
	Time updateTimerRelatedToStat(Timer& _timer, Time _diff, Time _baseDuration, Stat _stat)
	{
		auto diff = _diff;
		if (0 < _stat && 0 < _diff.count() && _timer.isActive() && 0 < _baseDuration.count())
		{
			if (auto newDuration = std::chrono::duration_cast<Timer::Duration>(_baseDuration / _stat);
				newDuration != _timer.getTimerTotal())
			{
				diff += std::chrono::duration_cast<Time>(_timer.getTimeElapsed());
				_timer.start(newDuration);
			}
		}
		return diff - std::chrono::duration_cast<Time>(_timer.update(diff));
	}
} // namespace game
