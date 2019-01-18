#include "stdafx.h"
#include "AuraEvent.hpp"

namespace game {
namespace aura {
namespace event {

	/*#####
	# Trigger
	#####*/
	Trigger::Trigger(Castable& _owner, Castable& _trigger, TriggerType _triggerType) :
		m_Owner(_owner),
		m_Trigger(_trigger),
		m_TriggerType(_triggerType)
	{
	}

	Castable& Trigger::getOwner() const
	{
		return m_Owner;
	}

	Castable& Trigger::getTrigger() const
	{
		return m_Trigger;
	}
	
	TriggerType Trigger::getTriggerType() const
	{
		return m_TriggerType;
	}

	EventType Trigger::getType() const
	{
		return EventType::trigger;
	}

	/*#####
	# CreateSpell
	#####*/
	CreateSpell::CreateSpell(Castable& _owner) :
		m_Owner(_owner)
	{
	}

	Castable& CreateSpell::getOwner() const
	{
		return m_Owner;
	}

	const std::vector<const data::SpellModifier*>& CreateSpell::getSpellMods() const
	{
		return m_SpellMods;
	}

	void CreateSpell::addSpellMod(const data::SpellModifier& _mod)
	{
		m_SpellMods.emplace_back(&_mod);
	}

	EventType CreateSpell::getType() const
	{
		return EventType::createSpell;
	}

	/*#####
	# IncomingSpell
	#####*/
	IncomingSpell::IncomingSpell(const Castable& _owner, CastableHandle _caster, ID _spellId) :
		m_Owner(_owner),
		m_Caster(std::move(_caster)),
		m_SpellId(_spellId)
	{
	}

	const Castable& IncomingSpell::getOwner() const
	{
		return m_Owner;
	}

	const CastableHandle& IncomingSpell::getCaster() const
	{
		return m_Caster;
	}

	ID IncomingSpell::getSpellId() const
	{
		return m_SpellId;
	}

	EventType IncomingSpell::getType() const
	{
		return EventType::incomingSpell;
	}

	void IncomingSpell::addBonus(int _bonus)
	{
		m_Bonus += _bonus;
	}

	int IncomingSpell::getBonus() const
	{
		return m_Bonus;
	}

	/*#####
	# CalculateDamage
	#####*/
	CalculateDamage::CalculateDamage(Castable& _owner, unit::DamageInfo& _damageInfo) :
		m_Owner(_owner),
		m_DamageInfo(_damageInfo)
	{
	}

	Castable& CalculateDamage::getOwner() const
	{
		return m_Owner;
	}

	unit::DamageInfo& CalculateDamage::getDamageInfo()
	{
		return m_DamageInfo;
	}

	EventType CalculateDamage::getType() const
	{
		return EventType::calculateDamage;
	}

	/*#####
	# AppliedXp
	#####*/
	AppliedXp::AppliedXp(Castable& _owner, Castable* _trigger, Stat _xpAmount, Reason _reason) :
		m_Owner(_owner),
		m_Trigger(_trigger),
		m_XpAmount(_xpAmount),
		m_Reason(_reason)
	{
	}

	Stat AppliedXp::getXpAmount() const
	{
		return m_XpAmount;
	}

	Castable& AppliedXp::getOwner() const
	{
		return m_Owner;
	}

	Castable* AppliedXp::getTrigger() const
	{
		return m_Trigger;
	}

	AppliedXp::Reason AppliedXp::getReason() const
	{
		return m_Reason;
	}

	EventType AppliedXp::getType() const
	{
		return EventType::applyXp;
	}
} // namespace event
} // namespace aura
} // namespace game
