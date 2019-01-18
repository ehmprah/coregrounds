#pragma once

namespace game
{
	class Level
	{
	private:
		Stat m_Progress = 0;
		const Stat XP_PER_LEVEL;
		const int MAX_LEVEL;
		uint32_t m_Level = 0;

		using XPCalculation = std::function<Stat(Stat)>;
		XPCalculation m_Calculation;

	public:
		Level(Stat _xpPerLevel, int _maxLevel, XPCalculation _calculation);

		uint32_t getProgressInPercent() const;
		uint32_t getLevel() const;
		uint32_t getXPTotal() const;
		Stat addXP(Stat _amount);
	};
} // namespace game
