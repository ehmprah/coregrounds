#include "Unit.hpp"
#include "Others.hpp"
#include "Core/HelperFunctions.hpp"
#include "Core/log.hpp"
#include "DataStringHelper.hpp"

namespace game::data
{
	void UnitModifiers::setModifier(Modifier _mod, Stat _value)
	{
		auto index = toInt(_mod) - 1;
		m_Values[index] = _value;
	}
	
	Stat UnitModifiers::getModifier(Modifier _mod) const
	{
		auto index = toInt(_mod) - 1;
		return m_Values[index];
	}

	Stat UnitModifiers::modifyValue(Modifier _mod, Stat _value) const
	{
		auto index = toInt(_mod) - 1;
		return _value * m_Values[index];
	}

	/*#####
	# Unit
	#####*/
	Unit::Stats& Unit::_validateStats(Stats& _stats) const
	{
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

	Unit::AuraIds& Unit::_validateAuraIds(AuraIds& _auras) const
	{
		_auras.erase(std::remove_if(std::begin(_auras), std::end(_auras),
			[this](auto _id)
			{
				if (_id <= 0)
				{
					LOG_WARN(getTypeName() << " id: " << *id << " invalid aura id:" << _id);
					return true;
				}
				return false;
			}
		), std::end(_auras));
		return _auras;
	}

	void Unit::setStats(Stats _stats)
	{
		m_Stats = std::move(_validateStats(_stats));
	}

	const Unit::Stats& Unit::getStats() const
	{
		return m_Stats;
	}

	void Unit::setXpBoostModifier(XpBoostModifier _mods)
	{
		m_XpBoostModifier = std::move(_mods);
	}

	const Unit::XpBoostModifier& Unit::getXpBoostModifier() const
	{
		return m_XpBoostModifier;
	}

	void Unit::setAuraIds(AuraIds _auraIds)
	{
		m_Auras = std::move(_validateAuraIds(_auraIds));
	}

	const Unit::AuraIds& Unit::getAuraIds() const
	{
		return m_Auras;
	}

	void Unit::setModifiers(UnitModifiers _mods)
	{
		m_Modifiers = std::move(_mods);
	}

	const UnitModifiers& Unit::getModifiers() const
	{
		return m_Modifiers;
	}

	void Unit::setBody(Shape _body)
	{
		m_Body = std::move(_body);
	}

	const Shape& Unit::getBody() const
	{
		return m_Body;
	}

	/*#####
	# Core
	#####*/
	std::string_view Core::getTypeName() const
	{
		return "Core";
	}

	/*#####
	# Wall
	#####*/
	std::string_view Wall::getTypeName() const
	{
		return "Wall";
	}

	/*#####
	# Tower
	#####*/
	std::string_view Tower::getTypeName() const
	{
		return "Tower";
	}

	/*#####
	# Minion
	#####*/
	void Minion::setFlags(Flags _flags)
	{
		m_Flags = _flags;
	}

	const Minion::Flags& Minion::getFlags() const
	{
		return m_Flags;
	}

	std::string_view Minion::getTypeName() const
	{
		return "Minion";
	}

	/*#####
	# Projectile
	#####*/
	void Projectile::setFlags(Flags _flags)
	{
		m_Flags = _flags;
	}

	Projectile::Flags Projectile::getFlags() const
	{
		return m_Flags;
	}

	std::string_view Projectile::getTypeName() const
	{
		return "Projectile";
	}

	/*#####
	# Trigger
	#####*/
	void Trigger::setFlags(Flags _flags)
	{
		m_Flags = _flags;
	}

	Trigger::Flags Trigger::getFlags() const
	{
		return m_Flags;
	}

	std::string_view Trigger::getTypeName() const
	{
		return "Trigger";
	}
} // namespace game::data
