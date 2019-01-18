#include "Device.hpp"
#include "Others.hpp"
#include "Core/log.hpp"
#include "DataStringHelper.hpp"

namespace game::data
{
	Device::Stats& Device::_validateStats(Stats& _stats) const
	{
		using device::StatType;
		for (std::size_t i = 1; i < static_cast<std::size_t>(StatType::max); ++i)
		{
			auto type = static_cast<StatType>(i);
			auto value = _stats.getStat(type);
			if (value < 0)
			{
				LOG_WARN(getTypeName() << " id: " << *id << " invalid stat :" << parseStatTypeToString(type) << " " << value << " Set to 0.");
				_stats.setStat(type, 0);
			}
		}
		return _stats;
	}

	void Device::setStats(Stats _stats)
	{
		m_Stats = std::move(_validateStats(_stats));
	}

	const Device::Stats& Device::getStats() const
	{
		return m_Stats;
	}

	void Device::setXpBoostModifier(XpBoostModifier _mods)
	{
		m_XpBoostModifier = std::move(_mods);
	}

	const Device::XpBoostModifier& Device::getXpBoostModifier() const
	{
		return m_XpBoostModifier;
	}
} // namespace game::data
