#include "stdafx.h"
#include "Level.hpp"

namespace game
{
	Level::Level(Stat _xpPerLevel, int _maxLevel, XPCalculation _calculation) :
		XP_PER_LEVEL(_xpPerLevel),
		MAX_LEVEL(_maxLevel),
		m_Calculation(_calculation)
	{
		assert(m_Calculation && XP_PER_LEVEL > 0 && _maxLevel >= 0);
	}

	uint32_t Level::getProgressInPercent() const
	{
		return m_Progress * 100 / XP_PER_LEVEL;
	}

	uint32_t Level::getLevel() const
	{
		return m_Level;
	}

	uint32_t Level::getXPTotal() const
	{
		return m_Level * XP_PER_LEVEL + m_Progress;
	}

	Stat Level::addXP(Stat _amount)
	{
		if (MAX_LEVEL <= m_Level || _amount < 0)
			return 0;
		auto calculated = m_Calculation(_amount);
		m_Progress += calculated;
		while (m_Progress >= XP_PER_LEVEL && m_Level < MAX_LEVEL)
		{
			++m_Level;
			m_Progress -= XP_PER_LEVEL;
		}
		return calculated;
	}
} // namespace game
