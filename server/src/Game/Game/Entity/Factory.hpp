#pragma once

#include "Device.hpp"
#include "GUIDManager.hpp"
#include "Level.hpp"
#include "Core/TargetPreference.hpp"
#include "Tower.hpp"

namespace game::device
{
	class Upgrades
	{
	private:
		int m_TotalUpgrades = 0;
		int m_UpgradesCombined = 0;
		std::optional<std::pair<int, int>> m_LastActivatedUpgrade;
		FactoryUpgrades m_Upgrades;

		void _calculateUpgradeProperty();

	public:
		Upgrades();

		// 1. index; 2. value
		std::optional<std::pair<int, int>> getLastActivatedUpgrade() const;

		bool upgrade(int _index);

		const FactoryUpgrades& getUpgrades() const;
		int getTotal() const;
		int getUpgradesCombined() const;

		static bool isValidIndex(int _index);
	};

	class Stack
	{
	private:
		Timer m_Timer;
		int m_Current = 0;

		using MaxStackCalculation = std::function<int()>;
		MaxStackCalculation m_MaxStackCalculation = nullptr;
		using SpawnSpeedCalculation = std::function<Stat()>;
		SpawnSpeedCalculation m_SpawnSpeedCalculation = nullptr;

	public:
		Stack(MaxStackCalculation _maxStackCalculation, SpawnSpeedCalculation _spawnSpeedCalculation);

		void update(Time _diff);
		void setStack(int _stack);
		void consume();
		void resetProgress();

		float getProgressPercent() const;
		int getCurrent() const;
	};

	struct FactoryInitializer
	{
		DeviceInitializer deviceInit;
		const data::Factory& factoryData;
	};

	class Factory :
		public Device
	{
	private:
		using super = Device;

	public:
		const data::Factory& factoryData;

		Factory(const FactoryInitializer& _initializer);
		virtual ~Factory() = default;

		const data::Factory& getFactoryData() const;
		virtual void serialize(protobuf::out::Match_Game_Factory& _msg) const;

		virtual bool hasActiveCooldown() const = 0;
		virtual Time getCooldown() const = 0;
		virtual uint32_t getCooldownProgressPercent() const = 0;

		virtual FactoryUpgrades getUpgrades() const = 0;
		virtual uint32_t getUpgradeTotal() const = 0;
		virtual Stat getXPTotal() const = 0;
		virtual int getStack() const = 0;

		virtual void setTargetPreference(const TargetPreference& _preference) = 0;

		virtual void activate(const AbsPosition& _destination) = 0;
		virtual void upgrade(int _index) = 0;

		virtual FactoryType getType() const = 0;
	};
	using FactoryPtr = std::unique_ptr<Factory>;

	class WallFactory :
		public Factory
	{
	private:
		using super = Factory;

		Stack m_Stack;

	public:
		WallFactory(const FactoryInitializer& _initializer);

		int getStack() const override;
		void setStack(int _stack);

		void setTargetPreference(const TargetPreference& _preference) override;
		void upgrade(int _index) override;
		void activate(const AbsPosition& _pos) override;
		void update(Time _diff) override;
		void serialize(protobuf::out::Match_Game_Factory& _msg) const override;
		bool hasActiveCooldown() const override;
		Time getCooldown() const override;
		uint32_t getCooldownProgressPercent() const override;

		FactoryUpgrades getUpgrades() const override;
		uint32_t getUpgradeTotal() const override;
		Stat getXPTotal() const override;

		FactoryType getType() const override;
	};

	class TowerFactory :
		public Factory
	{
	private:
		using super = Factory;

		unit::TowerHandle m_TowerHandle;
		TargetPreference m_TargetPreference;
		Level m_Level;

		Upgrades m_Upgrades;
		unit::TowerTimer m_TowerTimer;

	public:
		TowerFactory(const FactoryInitializer& _initializer);

		int getStack() const override;

		bool hasActiveCooldown() const override;
		Time getCooldown() const override;
		uint32_t getCooldownProgressPercent() const override;

		void setTargetPreference(const TargetPreference& _preference) override;
		void upgrade(int _index) override;
		void activate(const AbsPosition& _pos) override;
		void update(Time _diff) override;
		void serialize(protobuf::out::Match_Game_Factory& _msg) const override;;

		FactoryUpgrades getUpgrades() const override;
		std::uint32_t getUpgradeTotal() const override;
		Stat getXPTotal() const override;

		Stat calculateStat(StatType _type) const override;
		FactoryType getType() const override;
	};

	class MinionFactory :
		public Factory
	{
	private:
		using super = Factory;

		TargetPreference m_TargetPreference;
		Level m_Level;
		Upgrades m_Upgrades;
		Stack m_Stack;
		Timer m_UpgradeTimer;

		void _updateStacks(Time _diff);

	public:
		MinionFactory(const FactoryInitializer& _initializer);

		int getStack() const override;
		void setTargetPreference(const TargetPreference& _preference) override;
		void upgrade(int _index) override;
		void activate(const AbsPosition& _pos) override;
		void update(Time _diff) override;
		void serialize(protobuf::out::Match_Game_Factory& _msg) const override;
		bool hasActiveCooldown() const override;
		Time getCooldown() const override;
		uint32_t getCooldownProgressPercent() const override;

		FactoryUpgrades getUpgrades() const override;
		uint32_t getUpgradeTotal() const override;
		Stat getXPTotal() const override;

		Stat calculateStat(StatType _type) const override;
		FactoryType getType() const override;
	};

	FactoryPtr createFactory(const FactoryInitializer& _initializer);
} // namespace game::device
