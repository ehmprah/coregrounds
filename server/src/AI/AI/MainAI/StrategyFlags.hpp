#pragma once

#include <vector>
#include <algorithm>

namespace game::ai 
{
	enum class StrategyFlagType
	{
		SpawnWhenReady =			0x0001,
		MoveTowersToCore =			0x0002,
		DestroyWallsOffensively =	0x0004,
		UpgradeOnlyAtMinionLimit =	0x0008,
		BuildTowersAsap =			0x0010,
		NoEarlyUpgrades =			0x0020,
		EarlyMidGame =				0x0040,
		EarlyLateGame =				0x0080
	};

	using StrategyFlagMask = sl2::Bitmask<StrategyFlagType>;

	static StrategyFlagMask translateFlags(const std::vector<std::string>& _flags)
	{
		StrategyFlagMask bitmask;
		for (auto& flag : _flags)
		{
			if (flag == "SpawnWhenReady") bitmask.apply(StrategyFlagType::SpawnWhenReady);
			else if (flag == "MoveTowersToCore") bitmask.apply(StrategyFlagType::MoveTowersToCore);
			else if (flag == "DestroyWallsOffensively") bitmask.apply(StrategyFlagType::DestroyWallsOffensively);
			else if (flag == "UpgradeOnlyAtMinionLimit") bitmask.apply(StrategyFlagType::UpgradeOnlyAtMinionLimit);
			else if (flag == "BuildTowersAsap") bitmask.apply(StrategyFlagType::BuildTowersAsap);
			else if (flag == "NoEarlyUpgrades") bitmask.apply(StrategyFlagType::NoEarlyUpgrades);
			else if (flag == "EarlyMidGame") bitmask.apply(StrategyFlagType::EarlyMidGame);
			else if (flag == "EarlyLateGame") bitmask.apply(StrategyFlagType::EarlyLateGame);
		}
		return bitmask;
	}

	class StrategyFlags
	{
	public:
		StrategyFlags(StrategyFlagMask _flags) :
			m_Flags(_flags)
		{
		}

		bool hasFlag(StrategyFlagType _type) const
		{
			return m_Flags.contains(_type);
		}

	private:
		StrategyFlagMask m_Flags;
	};
}