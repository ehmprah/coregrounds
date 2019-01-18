#include "stdafx.h"
#include "Unit.hpp"
#include "Participant.hpp"
#include "Data/Game.hpp"
#include "Data/Unit.hpp"
#include "Minion.hpp"
#include "Team.hpp"
#include "Map/ColliderMap.hpp"
#include "Map/VisibilityMap.hpp"
#include "Spell/AuraEvent.hpp"
#include "Level.hpp"
#include "Core/HelperFunctions.hpp"

namespace game::unit
{
	Unit::Unit(const UnitInitializer& _initializer) :
		super(_initializer.entityInit),
		unitData(_initializer.unitData),
		m_Level(_initializer.level),
		m_MasterHandle(*this),
		m_Health(unitData.getStats().getStat(StatType::maxHealth))
	{
		m_UnitState.health = m_Health;	// this prevents the unit from being treated as a valid target for TargetPreferenceFlag::damaged
	}

	void Unit::derivedStartPlay()
	{
		super::derivedStartPlay();

		_applyInitAuras();
		_startRegenerationTimer();
		m_Health = calculateStat(StatType::maxHealth);

		_storeUnitState();
	}

	void Unit::_storeUnitState()
{
		UnitState unitState;
		unitState.visualIds = gatherAuraVisualIds();

		for (std::size_t i = 1; i < static_cast<std::size_t>(StatType::max); ++i)
		{
			auto type = static_cast<StatType>(i);
			unitState.stats.setStat(type, calculateStat(type));
		}
		unitState.conditions = m_Conditions;
		unitState.pos = getPosition();
		unitState.level = m_Level ? m_Level->getLevel() : 0;
		unitState.health = m_Health;
		unitState.isInWorld = m_IsInWorld;
		unitState.state = m_State;
		setupDerivedState(unitState);
		m_UnitState = std::move(unitState);
	}

	void Unit::derivedFinalizeTick()
	{
		super::derivedFinalizeTick();

		if (isAlive())
		{
			if ((m_Killed || m_Health <= 0) && canDie())
			{
				m_State = State::justDied;

				if (!m_Killed)
				{
					auto killer = m_LastAttackerInfo && m_LastAttackerInfo->owner ? m_LastAttackerInfo->owner.get() : nullptr;
					aura::event::Trigger triggerEvent(*this, killer ? *killer : *this, aura::TriggerType::died);
					notifyAuras(triggerEvent);

					if (m_LastAttackerInfo)
					{
						if (auto killerOwner = m_LastAttackerInfo->participant ? m_LastAttackerInfo->participant : nullptr; killerOwner)
							killerOwner->applyUnitKillToStatistics(getType());
						if (auto killXp = std::max<Stat>(m_LastAttackerInfo->unitModifiers ?
							m_LastAttackerInfo->unitModifiers->modifyValue(unit::Modifier::xpOnUnitKilled, *unitData.killingXp) : *unitData.killingXp, Stat(0));
						0 < killXp)
						{
							if (m_LastAttackerInfo->level)
								m_LastAttackerInfo->level->addXP(killXp);
							aura::event::AppliedXp event(*this, killer, killXp, aura::event::AppliedXp::Reason::kill);
							notifyAuras(event);
						}
					}
				}

				removeFromWorld();
			}
			else
				m_Health = std::min(m_Health, getUnitState().stats.getStat(StatType::maxHealth));
		}

		switch (getUnitState().state)
		{
		case State::justSpawned:
			if (m_State == State::justSpawned)
				m_State = State::alive;
			break;
		case State::justDied:
			m_State = State::dead;
			cleanUp();
			break;
		}
		_storeUnitState();
	}

	void Unit::setOwner(Participant& _owner)
	{
		if (&getOwner() != &_owner)
		{
			auto inWorld = m_IsInWorld;
			if (inWorld)
				removeFromWorld();
			getOwner().removeUnit(*this);
			super::setOwner(_owner);

			/* It is important to add the unit first to the world and after that to the owner, because otherwise it would
			corrupt the building map of all participants*/
			if (inWorld)
				addIntoWorld();
			getOwner().addUnit(*this);
			setTargetFinder(getOwner().getTargetFinder());
		}
	}

	UnitHandle Unit::getUnitHandle() const
	{
		return m_MasterHandle.get();
	}

	void Unit::addIntoWorld()
	{
		if (!m_IsInWorld)
		{
			m_ColliderGridEntity = &getGameDependencies().colliderMap.create(getCollider(), *this);
			m_IsInWorld = true;
			LOG_INFO(unitData.getTypeName() << " GUID: " << getGUID() << " entered world at: " << getPosition().getX() << "/" << getPosition().getY());
		}
	}

	void Unit::removeFromWorld()
	{
		if (m_IsInWorld)
		{
			assert(m_ColliderGridEntity);
			getGameDependencies().colliderMap.destroy(*m_ColliderGridEntity);
			m_ColliderGridEntity = nullptr;
			m_IsInWorld = false;
			LOG_INFO(unitData.getTypeName() << " GUID: " << getGUID() << " leaves world.");
		}
	}

	Stat Unit::calculateStat(StatType _type) const
	{
		Stat base = getStatBase(_type);
		Stat levelBoost = m_Level ? base * m_Level->getLevel() * unitData.getXpBoostModifier().getModifier(_type) : 0;
		return std::max<Stat>(0, statModifier.modifyValue(_type, base) + levelBoost);
	}

	Stat Unit::getStatBase(StatType _type) const
	{
		return unitData.getStats().getStat(_type);
	}

	Stat Unit::calculateVisibilityRange() const
	{
		return getUnitState().stats.getStat(StatType::visionRange);
	}

	void Unit::setHealthPercent(Stat _percent)
	{
		auto maxHp = getUnitState().stats.getStat(StatType::maxHealth);
		m_Health =  maxHp * _percent / 100;
	}

	void Unit::_applyInitAuras()
	{
		auto& auras = getGameDependencies().gameData.auras;
		auto casterInfo = generateCastableInfo();
		for (auto id : unitData.getAuraIds())
		{
			if (auto auraDef = data::findData(auras, id))
			{
				addAura(aura::createAura(*auraDef, *this, casterInfo));
				LOG_INFO(unitData.getTypeName() << " ID: " << getId() << " add init aura id: " << id);
			}
			else
				LOG_ERR(unitData.getTypeName() << " ID: " << getId() << " Init aura id: " << id << " not found.");
		}
	}

	void Unit::_startRegenerationTimer()
	{
		m_RegenerationTimer.start(*getGameDependencies().gameData.gameProperties.game.unitRegenerationInterval);
	}

	void Unit::_updateRegeneration(Time _diff)
	{
		m_RegenerationTimer.update(_diff);
		if (m_RegenerationTimer.getState() == Timer::State::justFinished)
		{
			auto& stats = getUnitState().stats;
			if (auto regenAmount = stats.getStat(StatType::maxHealth) * stats.getStat(StatType::regeneration);
				regenAmount > Stat(0))
			{
				m_Health += regenAmount;
				auto effectiveHealing = std::min(regenAmount, getUnitState().stats.getStat(StatType::maxHealth) - getUnitState().health);
				getOwner().applyHealToStatistics(effectiveHealing);
			}
			_startRegenerationTimer();
		}
	}

	void Unit::adjustCollider()
	{
		if (m_ColliderGridEntity)
			m_ColliderGridEntity->adjust();
	}

	GUID Unit::getGUID() const
	{
		return m_GUID;
	}

	void Unit::setUnitFlag(unit::Flags _flag, bool _set)
	{
		if (_set != m_Flags.contains(_flag))
		{
			_set ? m_Flags.apply(_flag) : m_Flags.remove(_flag);
			LOG_INFO(unitData.getTypeName() << " GUID: " << getGUID() << " set flag: " << static_cast<std::uint32_t>(_flag) << " to " << _set);
		}
	}

	bool Unit::hasUnitFlag(unit::Flags _flag) const
	{
		return m_Flags.contains(_flag);
	}

	void Unit::applyUnitCondition(Condition _condition, bool _set /*= true*/)
	{
		assert(_condition != Condition::none);
		m_Conditions[toInt(_condition) - 1] += _set ? 1 : -1;
		assert(m_Conditions[toInt(_condition) - 1] >= 0);
	}

	const Team& Unit::getTeam() const
	{
		return getOwner().getTeam();
	}

	ID Unit::getId() const
	{
		return *unitData.id;
	}

	bool Unit::isAlive() const
	{
		return m_State == unit::State::alive ||
			m_State == unit::State::justSpawned;
	}

	bool Unit::isDead() const
	{
		return !isAlive();
	}

	bool Unit::isErasable() const
	{
		return getUnitState().state == State::dead &&
			!isAffectedBySpell();
	}

	bool Unit::isCrowdControlled() const
	{
		return hasCondition(*this, Condition::root) || hasCondition(*this, Condition::daze) ||
			statModifier.hasNegativeMod(StatType::attackSpeed, StatModifierGroup::buff) ||
			statModifier.hasNegativeMod(StatType::moveSpeed, StatModifierGroup::buff);
	}

	bool Unit::isTargetable() const
	{
		return isAlive();
	}

	void Unit::update(Time _diff)
	{
		if (isDead())
			return;

		_updateRegeneration(_diff);
		super::update(_diff);
	}

	void Unit::kill(const CastableSnapshot& _killerInfo)
	{
		if (!hasCondition(*this, Condition::invulnerable))
		{
			m_Health = std::numeric_limits<Stat>::lowest();
			assert(_killerInfo.participant);
			_killerInfo.participant->applyUnitKillToStatistics(getType());
		}
	}

	void Unit::kill()
	{
		m_Killed = true;
	}

	Stat Unit::applyHealing(Stat _baseHealing, const CastableSnapshot& _dealerInfo)
	{
		auto value = std::max<Stat>(unitData.getModifiers().modifyValue(unit::Modifier::healGained, _baseHealing), 0);
		m_Health += value;
		assert(getUnitState().health <= getUnitState().stats.getStat(StatType::maxHealth));
		auto effectiveHealing = std::min(value, getUnitState().stats.getStat(StatType::maxHealth) - getUnitState().health);
		if (_dealerInfo.unitModifiers && _dealerInfo.level)
		{
			auto dealerXp = _dealerInfo.unitModifiers->modifyValue(unit::Modifier::xpOnHealingDealt, effectiveHealing);
			_dealerInfo.level->addXP(dealerXp);
		}

		if (_dealerInfo.participant)
			_dealerInfo.participant->applyHealToStatistics(effectiveHealing);
		return effectiveHealing;
	}

	void Unit::cleanUp()
	{
		clearAuras();
		getOwner().removeUnit(*this);
	}

	Stat Unit::_getDamageModifier(const data::UnitModifiers& _mods) const
	{
		Stat mod = 0;
		switch (getType())
		{
		case unit::Type::core:
			mod += _mods.getModifier(unit::Modifier::damageAgainstCores);
			break;
		case unit::Type::minion:
			mod += _mods.getModifier(unit::Modifier::damageAgainstMinions);
			break;
		case unit::Type::tower:
			mod += _mods.getModifier(unit::Modifier::damageAgainstTowers);
			break;
		case unit::Type::wall:
			mod += _mods.getModifier(unit::Modifier::damageAgainstWalls);
			break;
		}

		if (isCrowdControlled())
			mod += _mods.getModifier(unit::Modifier::damageAgainstCC);
		return mod;
	}

	Stat Unit::_shareDmgWithLifePool(const LifeLinkPool& _lifeLinkPool, Stat _damage, const CastableSnapshot& _dealerInfo)
	{
		if (_lifeLinkPool.isEmpty())
			return 0;

		auto sharedDmgList = _lifeLinkPool.calculateSharedDamage(_damage);
		Stat sharedDmg = 0;
		for (auto& el : sharedDmgList)
		{
			if (el.first != this)
			{
				sharedDmg += el.second;
				el.first->applyDirectDamage(el.second);
			}
		}
		return sharedDmg;
	}

	void Unit::_applyDealerXpAndStatistics(Stat _totalDamage, const CastableSnapshot& _dealerInfo)
	{
		if (_dealerInfo.participant)
			_dealerInfo.participant->applyDamageDealtToStatistics(_totalDamage);

		Stat totalXPDealer = 0;
		if (m_Level)
			m_Level->addXP(unitData.getModifiers().modifyValue(unit::Modifier::xpOnDamageTaken, _totalDamage));
		if (_dealerInfo.unitModifiers)
		{
			auto dmgXp = std::max<Stat>(_dealerInfo.unitModifiers->modifyValue(unit::Modifier::xpOnDamageDealt, _totalDamage), 0);
			totalXPDealer += dmgXp;
			aura::event::AppliedXp event(*this, _dealerInfo.owner ? _dealerInfo.owner.get() : nullptr, dmgXp, aura::event::AppliedXp::Reason::damage);
			notifyAuras(event);
		}

		if (_dealerInfo.level && totalXPDealer > 0)
			_dealerInfo.level->addXP(totalXPDealer);

		m_LastAttackerInfo = _dealerInfo;
	}

	Stat Unit::applyDamage(Stat _baseDamage, const CastableSnapshot& _dealerInfo, bool _ignoreLifeLinkAndThorns)
	{
		if (_baseDamage <= 0 || isDead() || hasCondition(*this, Condition::invulnerable))
			return 0;

		DamageInfo dmgInfo;
		dmgInfo.plainDamage = _baseDamage;

		// calculate armor reduction
		auto armorStat = getUnitState().stats.getStat(StatType::armor);
		auto incArmorPen = _dealerInfo.unitStats.getStat(StatType::armorPenetration);
		auto armor = std::max(Stat(0), armorStat - incArmorPen);
		dmgInfo.damageAfterArmor = dmgInfo.plainDamage * Stat(100) / (Stat(100) + armor);

		dmgInfo.damageAfterDmgTaken = getUnitState().stats.getStat(StatType::damageTaken) * dmgInfo.damageAfterArmor;

		// apply damage modifiers if possible
		Stat modifier = 0;
		if (_dealerInfo.unitModifiers)
			modifier = _getDamageModifier(*_dealerInfo.unitModifiers);
		dmgInfo.damageAfterModifiers = std::max(Stat(0), dmgInfo.damageAfterDmgTaken * (1 + modifier));

		aura::event::CalculateDamage event(*this, dmgInfo);
		notifyAuras(event);
		dmgInfo.totalDamage = dmgInfo.damageAfterModifiers - dmgInfo.absorbedDamage;
		if (!_ignoreLifeLinkAndThorns)
		{
			dmgInfo.sharedDamage = _shareDmgWithLifePool(dmgInfo.lifeLinkPool, dmgInfo.totalDamage, _dealerInfo);
			if (_dealerInfo.owner && _dealerInfo.owner->isUnit())
			{
				dmgInfo.thornsDamage = std::max<Stat>(0, getUnitState().stats.getStat(StatType::thorns) * dmgInfo.totalDamage);
				auto& unitDealer = _dealerInfo.owner->toUnit();
				dmgInfo.thornsDamage = unitDealer.applyDamage(dmgInfo.thornsDamage, generateCastableInfo(), true);
			}
		}
		assert(dmgInfo.totalDamage >= dmgInfo.sharedDamage);
		dmgInfo.finalDamage = dmgInfo.totalDamage - dmgInfo.sharedDamage;
		applyDamageWithoutModifiers(dmgInfo.finalDamage, _dealerInfo);
		return dmgInfo.finalDamage;
	}

	void Unit::applyDamageWithoutModifiers(Stat _damage, const CastableSnapshot& _dealerInfo)
	{
		if (isAlive())
		{
			applyDirectDamage(_damage);
			_applyDealerXpAndStatistics(_damage, _dealerInfo);
		}
	}

	void Unit::applyDirectDamage(Stat _damage)
	{
		if (_damage <= 0 || isDead())
			return;
		assert(_damage >= 0);
		m_Health -= _damage;
	}

	bool Unit::canCast() const
	{
		return isAlive() && !hasCondition(*this, Condition::silence);
	}

	void Unit::setupCastableSnapshot(CastableSnapshot& _info) const
	{
		super::setupCastableSnapshot(_info);

		_info.health = getUnitState().health;
		_info.unitStats = getUnitState().stats;
		_info.unitModifiers = &unitData.getModifiers();
		_info.level = m_Level;
		_info.targetPreference = getTargetPreference();
	}

	unit::Minion* Unit::spawnMinion(ID _id, const AbsPosition& _pos)
	{
		return getOwner().createMinion(_id, _pos, getTargetPreference(), m_Level);
	}

	unit::Trigger* Unit::spawnTrigger(ID _id, const AbsPosition& _pos)
	{
		return getOwner().createTrigger(_id, _pos);
	}

	bool Unit::isParticipant() const
	{
		return false;
	}

	bool Unit::isDevice() const
	{
		return false;
	}

	const device::Device& Unit::toDevice() const
	{
		throw std::runtime_error("Castable: Is no Device type.");
	}

	device::Device& Unit::toDevice()
	{
		throw std::runtime_error("Castable: Is no Device type.");
	}

	bool Unit::isUnit() const
	{
		return true;
	}

	const unit::Unit& Unit::toUnit() const
	{
		return *this;
	}

	unit::Unit& Unit::toUnit()
	{
		return *this;
	}

	const TargetPreference* Unit::getTargetPreference() const
	{
		return nullptr;
	}

	const UnitState& Unit::getUnitState() const
	{
		return m_UnitState;
	}

	bool Unit::isInWorld() const
	{
		return m_IsInWorld;
	}

	bool Unit::canDie() const
	{
		return true;
	}

	unit::Unit& toUnit(CastableHandle& _castable)
	{
		assert(_castable.isValid() && _castable->isUnit());
		return _castable->toUnit();
	}

	float getHealthPercent(const UnitState& _unitState)
	{
		return std::clamp(_unitState.health / _unitState.stats.getStat(StatType::maxHealth) * 100, 0.f, 100.f);
	}

	bool hasCondition(const Unit& _unit, Condition _condition)
	{
		assert(_condition != Condition::none);
		return _unit.getUnitState().conditions[toInt(_condition) - 1] > 0;
	}

	bool isAlive(const Unit& _unit)
	{
		return _unit.getUnitState().state == unit::State::alive ||
			_unit.getUnitState().state == unit::State::justSpawned;
	}

	bool isDead(const Unit& _unit)
	{
		return !isAlive(_unit);
	}

	bool isInWorld(const Unit& _unit)
	{
		return _unit.getUnitState().isInWorld;
	}

	bool belongToSameOwner(const Unit& _lhs, const Unit& _rhs)
	{
		return &_lhs.getOwner() == &_rhs.getOwner();
	}

	bool isFriendly(const Unit& _unit, const Unit& _other)
	{
		return _unit.getTeam() == _other.getTeam();
	}

	bool isFriendly(const Unit& _unit, const Participant& _participant)
	{
		return _unit.getTeam() == _participant.getTeam();
	}

	bool isHostile(const Unit& _unit, const Unit& _other)
	{
		return !isFriendly(_unit, _other);
	}

	bool isHostile(const Unit& _unit, const Participant& _participant)
	{
		return !isFriendly(_unit, _participant);
	}
} // namespace game::entity::unit
