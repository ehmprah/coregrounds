#include "stdafx.h"
#include "Factory.hpp"
#include "Tower.hpp"
#include "Participant.hpp"
#include "Wall.hpp"
#include "Minion.hpp"
#include "Game/Spell/AuraEvent.hpp"
#include "Data/Game.hpp"

namespace game::device
{
	FactoryPtr createFactory(const FactoryInitializer& _initializer)
	{
		if (_initializer.factoryData.hasData<data::Factory::Wall>())
			return std::make_unique<WallFactory>(_initializer);
		else if (_initializer.factoryData.hasData<data::Factory::Minion>())
			return std::make_unique<MinionFactory>(_initializer);
		else if (_initializer.factoryData.hasData<data::Factory::Tower>())
			return std::make_unique<TowerFactory>(_initializer);
		throw std::runtime_error("CreateFactory: ID: " + std::to_string(*_initializer.factoryData.id) + " Invalid type.");
	}

	uint32_t _targetPreferenceToInt(const TargetPreference& _targetPreference)
	{
		return static_cast<uint32_t>(*_targetPreference.target) | static_cast<uint32_t>(_targetPreference.mode) << 15;
	}

	Stat _modifyIncomingXP(const Factory& _factory, uint32_t _level, Stat _value, uint32_t _upgradeTotal)
	{
		auto& gameData = _factory.getGameDependencies().gameData;
		auto modPercent = gameData.gameProperties.game.xpCurve[_level];
		modPercent -= _upgradeTotal * *gameData.gameProperties.game.xpHandicapPerUpgrade;
		return std::max<Stat>(0, (_value + _value * modPercent) * _factory.calculateStat(StatType::xp));
	}

	/*#####
	# Upgrades
	#####*/
	Time _calculateTimerForUpgrades(int _upgradeTotal)
	{
		assert(0 <= _upgradeTotal);
		return Time(_upgradeTotal * 5000 + 5000);		// 5sec base + 5sec for each upgrade
	}

	Upgrades::Upgrades()
	{
		m_Upgrades.fill(0);
	}

	std::optional<std::pair<int, int>> Upgrades::getLastActivatedUpgrade() const
	{
		return m_LastActivatedUpgrade;
	}

	void Upgrades::_calculateUpgradeProperty()
	{
		uint32_t i = 0;
		m_UpgradesCombined = 0;
		// store each upgrade in 3 bits
		for (auto upgrade : m_Upgrades)
		{
			assert(upgrade <= std::pow(2, 3));
			m_UpgradesCombined |= upgrade << i;
			i += 3;
		}
	}

	bool Upgrades::upgrade(int _index)
	{
		if (isValidIndex(_index))
		{
			auto min = std::min_element(std::begin(m_Upgrades), std::end(m_Upgrades));
			if (m_Upgrades[_index] < MaxFactoryUpgrades && m_Upgrades[_index] < *min + 2)
			{
				m_LastActivatedUpgrade = std::make_pair(_index, static_cast<int>(m_Upgrades[_index]));
				++m_Upgrades[_index];
				++m_TotalUpgrades;
				_calculateUpgradeProperty();
				return true;
			}
		}
		return false;
	}

	const FactoryUpgrades& Upgrades::getUpgrades() const
	{
		return m_Upgrades;
	}

	int Upgrades::getTotal() const
	{
		assert(0 <= m_TotalUpgrades);
		return m_TotalUpgrades;
	}

	int Upgrades::getUpgradesCombined() const
	{
		assert(0 <= m_UpgradesCombined);
		return m_UpgradesCombined;
	}

	bool Upgrades::isValidIndex(int _index)
	{
		return _index >= 0 && _index < MaxFactoryUpgradeTypes;
	}

	/*#####
	# Stack
	#####*/
	Stack::Stack(MaxStackCalculation _maxStackCalculation, SpawnSpeedCalculation _spawnSpeedCalculation) :
		m_MaxStackCalculation(_maxStackCalculation),
		m_SpawnSpeedCalculation(_spawnSpeedCalculation)
	{
		assert(m_MaxStackCalculation && _spawnSpeedCalculation);
		m_Timer.start(SpawnSpeedBase);
	}

	void Stack::update(Time _diff)
	{
		auto spawnSpeed = m_SpawnSpeedCalculation();
		if (spawnSpeed <= 0)
			return;
		int diff = std::chrono::duration_cast<std::chrono::microseconds>(_diff).count() * spawnSpeed;
		auto maxStacks = m_MaxStackCalculation();
		while (m_Current < maxStacks && diff > 0 && m_Timer.isActive())
		{
			auto old = m_Timer.getTimerValue().count();
			m_Timer.update(std::chrono::microseconds(diff));
			diff -= old - m_Timer.getTimerValue().count();
			if (!m_Timer.isActive())
			{
				++m_Current;
				m_Timer.start(SpawnSpeedBase);
			}
		}
	}

	void Stack::setStack(int _stack)
	{
		m_Current = _stack;
	}

	void Stack::consume()
	{
		assert(m_Current > 0);
		--m_Current;
		if (!m_Timer.isActive() && m_Current < m_MaxStackCalculation())
			m_Timer.start(SpawnSpeedBase);
	}

	void Stack::resetProgress()
	{
		m_Timer.restart();
	}

	float Stack::getProgressPercent() const
	{
		return m_Timer.getProgressInPercent();
	}

	int Stack::getCurrent() const
	{
		return m_Current;
	}

	/*#####
	# Factory
	#####*/
	Factory::Factory(const FactoryInitializer& _initializer) :
		super(_initializer.deviceInit),
		factoryData(_initializer.factoryData)
	{
	}

	const data::Factory& Factory::getFactoryData() const
	{
		return factoryData;
	}

	void Factory::serialize(protobuf::out::Match_Game_Factory& _msg) const
	{
		_msg.set_id(setHighestBit(*factoryData.id));

#ifndef NDEBUG
		for (std::size_t i = static_cast<std::size_t>(StatType::none) + 1; i < static_cast<std::size_t>(StatType::max); ++i)
			_msg.add_stats(getStat(*this, static_cast<StatType>(i)));
#endif

		auto& visIdMsg = *_msg.mutable_auravisualids();
		visIdMsg.set_isset(true);
		for (auto id : gatherAuraVisualIds())
			visIdMsg.add_ids(id);
	}

	/*#####
	# WallFactory
	#####*/
	WallFactory::WallFactory(const FactoryInitializer& _initializer) :
		super(_initializer),
		// stack constructor
		m_Stack([this]() {
		return static_cast<uint32_t>(calculateStat(StatType::stacks));
	},
			[this]() {
		return calculateStat(StatType::spawnSpeed);
	})
	{
		assert(factoryData.hasData<data::Factory::Wall>());
	}

	int WallFactory::getStack() const
	{
		return m_Stack.getCurrent();
	}

	void WallFactory::setStack(int _stack)
	{
		m_Stack.setStack(_stack);
	}

	void WallFactory::setTargetPreference(const TargetPreference& _preference)
	{
		// nothing to do here
	}

	void WallFactory::activate(const AbsPosition& _pos)
	{
		if (hasActiveCooldown() || m_Stack.getCurrent() < 1)
			return;

		auto& wallFactory = factoryData.getData<data::Factory::Wall>();
		if (auto wall = getOwner().createWall(*wallFactory.id, mapToTilePosition(_pos)))
		{
			m_Stack.consume();
			aura::event::Trigger event(*this, *wall, aura::TriggerType::summonedUnit);
			notifyAuras(event);
		}
	}

	void WallFactory::update(Time _diff)
	{
		super::update(_diff);

		m_Stack.update(_diff);
	}

	void WallFactory::upgrade(int _index)
	{
		// nothing to do here
	}

	void WallFactory::serialize(protobuf::out::Match_Game_Factory& _msg) const
	{
		super::serialize(_msg);

		auto& msg = *_msg.mutable_wall();

		auto& wallFactory = factoryData.getData<data::Factory::Wall>();
		uint32_t progress = m_Stack.getProgressPercent();
		auto maxStacks = static_cast<uint32_t>(calculateStat(device::StatType::stacks));
		assert(m_Stack.getCurrent() < std::pow(2, 7) && maxStacks < std::pow(2, 7) && progress < std::pow(2, 7));
		msg.set_stacks(setHighestBit(m_Stack.getCurrent() | maxStacks << 7 | progress << 14));
	}

	bool WallFactory::hasActiveCooldown() const
	{
		return false;
	}

	Time WallFactory::getCooldown() const
	{
		return Time::zero();
	}

	uint32_t WallFactory::getCooldownProgressPercent() const
	{
		return 0;
	}

	FactoryUpgrades WallFactory::getUpgrades() const
	{
		return FactoryUpgrades();
	}

	uint32_t WallFactory::getUpgradeTotal() const
	{
		return 0;
	}

	Stat WallFactory::getXPTotal() const
	{
		return 0;
	}

	FactoryType WallFactory::getType() const
	{
		return FactoryType::wall;
	}

	/*#####
	# TowerFactory
	#####*/
	TowerFactory::TowerFactory(const FactoryInitializer& _initializer) :
		super(_initializer),
		m_TowerTimer(*this),
		m_Level(*getGameDependencies().gameData.gameProperties.game.xpPerLevel, *getGameDependencies().gameData.gameProperties.game.maxLevel,
			[this](Stat _value) { return _modifyIncomingXP(*this, m_Level.getLevel(), _value, m_Upgrades.getTotal()); }
		)
	{
		assert(factoryData.hasData<data::Factory::Tower>());
		m_TargetPreference = factoryData.getData<data::Factory::Tower>().getTargetPreference();
	}

	void TowerFactory::setTargetPreference(const TargetPreference& _preference)
	{
		if (_preference.mode <= unit::targetPreference::Mode::none || _preference.mode >= unit::targetPreference::Mode::max)
			throw std::runtime_error("TowerFactory: invalid target preference mode: " + std::to_string(toInt(_preference.mode)));
		if (*_preference.target < static_cast<uint32_t>(unit::targetPreference::Target::none) || *_preference.target >= static_cast<uint32_t>(unit::targetPreference::Target::max))
			throw std::runtime_error("TowerFactory: invalid target preference unit type: " + std::to_string(*_preference.target));
		m_TargetPreference = _preference;
	}

	void TowerFactory::activate(const AbsPosition& _pos)
	{
		if (hasActiveCooldown())
			return;

		// spawn tower
		if (!m_TowerHandle)
		{
			if (auto tower = getOwner().createTower(*factoryData.getData<data::Factory::Tower>().id, mapToTilePosition(_pos),
				m_TowerTimer, &m_TargetPreference, &m_Level))
			{
				m_TowerHandle = tower->getHandle();
				m_TowerTimer.registerTower(m_TowerHandle);
				m_TowerTimer.emitBuildCooldown(_calculateTimerForUpgrades(m_Upgrades.getTotal()));
				aura::event::Trigger event(*this, *tower, aura::TriggerType::summonedUnit);
				notifyAuras(event);
			}
		}
		// move tower
		else
		{
			auto& tower = *m_TowerHandle;
			tower.activateMove(mapToTilePosition(_pos));
		}
	}

	void TowerFactory::upgrade(int _index)
	{
		if (!hasActiveCooldown() && m_Upgrades.upgrade(_index))
		{
			m_TowerTimer.emitUpgradeCooldown(_calculateTimerForUpgrades(m_Upgrades.getTotal() - 1));
			if (m_TowerHandle)
				m_TowerHandle->restartAttack();
		}
	}

	void TowerFactory::update(Time _diff)
	{
		super::update(_diff);

		auto& fTowerData = factoryData.getData<data::Factory::Tower>();
		m_TowerTimer.update(_diff);
		if (m_TowerTimer.getUpgradeCooldown().getState() == Timer::State::justFinished)
		{
			if (auto upgradeInfo = m_Upgrades.getLastActivatedUpgrade())
			{
				auto [index, value] = *upgradeInfo;
				auto spellId = fTowerData.getUpgrades().spellIDs[index][value];
				if (auto spellInfo = findData(getGameDependencies().gameData.spells, spellId))
					castTriggered(*spellInfo, generateCastableInfo(), getCastableHandle());
				else
					LOG_ERR("TowerFactory: Upgrade Spell not found. Index: " << index << " upgrade: " << value << " spellID: " << spellId);

				// if we already built a tower, we force the upgrade apply on it
				if (m_TowerHandle)
				{
					aura::event::Trigger event(*this, *m_TowerHandle, aura::TriggerType::summonedUnit);
					notifyAuras(event);
				}
			}
		}

		// add xp per second
		m_Level.addXP(*fTowerData.xpPerSecond * _diff.count() / 1000);
	}

	void TowerFactory::serialize(protobuf::out::Match_Game_Factory& _msg) const
	{
		super::serialize(_msg);

		auto& msg = *_msg.mutable_tower();
		if (m_TowerHandle)
			msg.set_guid(m_TowerHandle->getGUID());
		assert(m_Level.getLevel() >= 0 && m_Level.getProgressInPercent() >= 0);

		msg.set_level(setHighestBit(m_Level.getLevel() | (m_Level.getProgressInPercent() << 15)));
		msg.set_cooldown(setHighestBit(static_cast<uint32_t>(getCooldown().count() / 100) | (getCooldownProgressPercent() << 16)));
		msg.set_upgrades(setHighestBit(m_Upgrades.getUpgradesCombined()));
		msg.set_targetpreference(setHighestBit(_targetPreferenceToInt(m_TargetPreference)));
	}

	bool TowerFactory::hasActiveCooldown() const
	{
		return unit::hasCooldown(m_TowerTimer);
	}

	Time TowerFactory::getCooldown() const
	{
		return unit::getRemainingTime(m_TowerTimer);
	}

	std::uint32_t TowerFactory::getCooldownProgressPercent() const
	{
		return unit::getProgressInPercent(m_TowerTimer);
	}

	FactoryUpgrades TowerFactory::getUpgrades() const
	{
		return m_Upgrades.getUpgrades();
	}

	uint32_t TowerFactory::getUpgradeTotal() const
	{
		return m_Upgrades.getTotal();
	}

	Stat TowerFactory::getXPTotal() const
	{
		return m_Level.getXPTotal();
	}

	Stat TowerFactory::calculateStat(StatType _type) const
	{
		Stat base = getStatBase(_type);
		Stat levelBoost = base * m_Level.getLevel() * factoryData.getXpBoostModifier().getModifier(_type);
		return statModifier.modifyValue(_type, base) + levelBoost;
	}

	FactoryType TowerFactory::getType() const
	{
		return FactoryType::tower;
	}

	int TowerFactory::getStack() const
	{
		return 0;
	}

	/*#####
	# MinionFactory
	#####*/
	MinionFactory::MinionFactory(const FactoryInitializer& _initializer) :
		super(_initializer),
		m_Level(*getGameDependencies().gameData.gameProperties.game.xpPerLevel, *getGameDependencies().gameData.gameProperties.game.maxLevel,
			[this](Stat _value){ return _modifyIncomingXP(*this, m_Level.getLevel(), _value, m_Upgrades.getTotal()); }
		),
		m_Stack([this]() { return static_cast<int>(calculateStat(StatType::stacks)); },
			[this]() { return calculateStat(StatType::spawnSpeed); }
		)
	{
		assert(factoryData.hasData<data::Factory::Minion>());
		m_TargetPreference = factoryData.getData<data::Factory::Minion>().getTargetPreference();
	}

	int MinionFactory::getStack() const
	{
		return m_Stack.getCurrent();
	}

	void MinionFactory::setTargetPreference(const TargetPreference& _preference)
	{
		if (_preference.mode <= unit::targetPreference::Mode::none || _preference.mode >= unit::targetPreference::Mode::max)
			throw std::runtime_error("MinionFactory: invalid target preference mode: " + std::to_string(toInt(_preference.mode)));
		if (*_preference.target < static_cast<std::uint32_t>(unit::targetPreference::Target::none) || *_preference.target >= static_cast<std::uint32_t>(unit::targetPreference::Target::max))
			throw std::runtime_error("MinionFactory: invalid target preference unit type: " + std::to_string(*_preference.target));
		m_TargetPreference = _preference;
	}

	void MinionFactory::upgrade(int _index)
	{
		if (!m_UpgradeTimer.isActive() && m_Upgrades.upgrade(_index))
		{
			m_Stack.resetProgress();
			m_UpgradeTimer.start(_calculateTimerForUpgrades(m_Upgrades.getTotal() - 1));
		}
	}

	void MinionFactory::activate(const AbsPosition& _pos)
	{
		if (hasActiveCooldown() || m_Stack.getCurrent() < 1)
			return;

		if (auto minion = getOwner().createMinion(*factoryData.getData<data::Factory::Minion>().id, getOwner().getCorePosition(), &m_TargetPreference, &m_Level))
		{
			m_Stack.consume();
			aura::event::Trigger event(*this, *minion, aura::TriggerType::summonedUnit);
			Castable::notifyAuras(event);
		}
	}

	void MinionFactory::update(Time _diff)
	{
		super::update(_diff);

		if (!m_UpgradeTimer.isActive())
			m_Stack.update(_diff);

		auto& fMinionData = factoryData.getData<data::Factory::Minion>();
		if (auto upgradeCount = m_Upgrades.getTotal(); 0 < upgradeCount)
		{
			updateTimerRelatedToStat(m_UpgradeTimer, _diff, _calculateTimerForUpgrades(upgradeCount - 1), getStat(*this, StatType::cooldownSpeed));
			if (m_UpgradeTimer.getState() == Timer::State::justFinished)
			{
				if (auto upgradeInfo = m_Upgrades.getLastActivatedUpgrade())
				{
					auto [index, value] = *upgradeInfo;
					auto spellId = fMinionData.getUpgrades().spellIDs[index][value];
					if (auto spellInfo = findData(getGameDependencies().gameData.spells, spellId))
						castTriggered(*spellInfo, generateCastableInfo(), getCastableHandle());
					else
						LOG_ERR("MinionFactory: Upgrade Spell not found. Index: " << index << " upgrade: " << value << " spellID: " << spellId);
				}
			}
		}

		// add xp per second
		m_Level.addXP(*fMinionData.xpPerSecond * _diff.count() / 1000);
	}

	void MinionFactory::serialize(protobuf::out::Match_Game_Factory& _msg) const
	{
		super::serialize(_msg);

		auto& msg = *_msg.mutable_minion();

		// update stack prop
		auto& minionFactory = factoryData.getData<data::Factory::Minion>();
		uint32_t progress = m_Stack.getProgressPercent();
		auto maxStacks = static_cast<uint32_t>(calculateStat(device::StatType::stacks));
		assert(m_Stack.getCurrent() < std::pow(2, 7) && maxStacks < std::pow(2, 7) && progress < std::pow(2, 7));
		msg.set_stacks(setHighestBit(m_Stack.getCurrent() | maxStacks << 7 | progress << 14));

		assert(m_Level.getLevel() >= 0 && m_Level.getProgressInPercent() >= 0);
		msg.set_level(setHighestBit(m_Level.getLevel() | (m_Level.getProgressInPercent() << 15)));
		msg.set_targetpreference(setHighestBit(_targetPreferenceToInt(m_TargetPreference)));
		msg.set_cooldown(setHighestBit(static_cast<uint32_t>(getCooldown().count() / 100) | (getCooldownProgressPercent() << 16)));
		msg.set_upgrades(setHighestBit(m_Upgrades.getUpgradesCombined()));
	}

	bool MinionFactory::hasActiveCooldown() const
	{
		return m_UpgradeTimer.isActive();
	}

	Time MinionFactory::getCooldown() const
	{
		if (m_UpgradeTimer.isActive())
			return m_UpgradeTimer.getTimerValueInMsec();
		return Time::zero();
	}

	uint32_t MinionFactory::getCooldownProgressPercent() const
	{
		if (m_UpgradeTimer.isActive())
			return m_UpgradeTimer.getProgressInPercent();
		return 0;
	}

	FactoryUpgrades MinionFactory::getUpgrades() const
	{
		return m_Upgrades.getUpgrades();
	}

	uint32_t MinionFactory::getUpgradeTotal() const
	{
		return m_Upgrades.getTotal();
	}

	Stat MinionFactory::getXPTotal() const
	{
		return m_Level.getXPTotal();
	}

	Stat MinionFactory::calculateStat(StatType _type) const
	{
		Stat base = getStatBase(_type);
		Stat levelBoost = base * m_Level.getLevel() * factoryData.getXpBoostModifier().getModifier(_type);
		return statModifier.modifyValue(_type, base) + levelBoost;
	}

	FactoryType MinionFactory::getType() const
	{
		return FactoryType::minion;
	}
} // namespace game::device
