#include "Factory.hpp"
#include "Core/log.hpp"

namespace game::data
{
	bool Factory::isEmpty() const
	{
		return std::holds_alternative<std::monostate>(m_Data);
	}

	std::string_view Factory::getTypeName() const
	{
		return "Factory";
	}

	Factory::Upgrades& _validateUpgrades(Factory::Upgrades& _upgrades)
	{
		for (std::size_t upIndex = 0; upIndex < device::MaxFactoryUpgradeTypes; ++upIndex)
		{
			if (_upgrades.names[upIndex].empty())
				LOG_WARN("Upgrade name is empty at: " << upIndex);
			for (std::size_t index = 0; index < device::MaxFactoryUpgrades; ++index)
			{
				if (_upgrades.spellIDs[upIndex][index] <= 0)
				{
					LOG_ERR("Invalid upgrades at index: " << upIndex << " name: " << _upgrades.names[upIndex] << " at: " << index << " Set to 0.");
					_upgrades.spellIDs[upIndex][index] = 0;
				}
			}
		}
		return _upgrades;
	}

	TargetPreference& _validateTargetPreference(TargetPreference& _pref)
	{
		using Mode = unit::targetPreference::Mode;
		if (_pref.mode <= Mode::none || _pref.mode >= Mode::max)
		{
			LOG_ERR("Invalid target_preference mode. Set to default.");
			_pref.mode = static_cast<Mode>(1);
		}

		using Target = unit::targetPreference::Target;
		if (*_pref.target <= 0)
		{
			LOG_ERR("Invalid target_preference target value: " << *_pref.target << " Set to all.");
			_pref.target.apply(Target::all);
		}
		return _pref;
	}

	/*#####
	# Factor::Minion
	#####*/
	void Factory::Minion::setUpgrades(Upgrades _upgrades)
	{
		m_Upgrades = std::move(_validateUpgrades(_upgrades));
	}

	const Factory::Upgrades& Factory::Minion::getUpgrades() const
	{
		return m_Upgrades;
	}

	void Factory::Minion::setTargetPreference(TargetPreference _pref)
	{
		m_TargetPreference = std::move(_validateTargetPreference(_pref));
	}

	const TargetPreference& Factory::Minion::getTargetPreference() const
	{
		return m_TargetPreference;
	}

	/*#####
	# Factor::Tower
	#####*/
	void Factory::Tower::setUpgrades(Upgrades _upgrades)
	{
		m_Upgrades = std::move(_validateUpgrades(_upgrades));
	}

	const Factory::Upgrades& Factory::Tower::getUpgrades() const
	{
		return m_Upgrades;
	}

	void Factory::Tower::setTargetPreference(TargetPreference _pref)
	{
		m_TargetPreference = std::move(_validateTargetPreference(_pref));
	}

	const TargetPreference& Factory::Tower::getTargetPreference() const
	{
		return m_TargetPreference;
	}
} // namespace game::data
