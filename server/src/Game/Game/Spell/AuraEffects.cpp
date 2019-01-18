#include "stdafx.h"
#include "AuraEffects.hpp"
#include "Game/Entity/Unit.hpp"
#include "Game/Entity/Device.hpp"
#include "Game/Entity/Level.hpp"
#include "Data/Game.hpp"
#include "AuraEvent.hpp"
#include "Aura.hpp"

namespace game::aura::effect
{
	EffectPtr createEffect(const data::Aura::Effect& _effect)
	{
		using Effect = data::Aura::Effect;
		switch (_effect.type)
		{
		case EffectType::dummy:
			return std::make_shared<Dummy>();
		case EffectType::offender:
			return std::make_shared<Offender>(_effect.getData<Effect::Offender>());
		case EffectType::lifeLink:
			return std::make_shared<LifeLink>();
		case EffectType::modIncSpell:
			return std::make_shared<ModIncSpellEffectivity>(_effect.getData<Effect::ModIncSpellEffectivity>());
		case EffectType::modSpell:
			return std::make_shared<ModifySpell>(_effect.getData<Effect::SpellModifier>());
		case EffectType::modStat:
			return std::make_shared<ModStat>(_effect.getData<Effect::ModStat>(), false);
		case EffectType::modStatPercent:
			return std::make_shared<ModStat>(_effect.getData<Effect::ModStat>(), true);
		case EffectType::periodicDamage:
			return std::make_shared<PeriodicDealing>(_effect.getData<Effect::PeriodicDealing>(), true, false);
		case EffectType::periodicDamagePercent:
			return std::make_shared<PeriodicDealing>(_effect.getData<Effect::PeriodicDealing>(), true, true);
		case EffectType::periodicHealing:
			return std::make_shared<PeriodicDealing>(_effect.getData<Effect::PeriodicDealing>(), false, false);
		case EffectType::periodicHealingPercent:
			return std::make_shared<PeriodicDealing>(_effect.getData<Effect::PeriodicDealing>(), false, true);
		case EffectType::condition:
			return std::make_shared<ApplyCondition>(_effect.getData<Effect::ApplyCondition>());
		case EffectType::triggerSpell:
			return std::make_shared<TriggerSpell>(_effect.getData<Effect::TriggerSpell>());
		case EffectType::periodicTrigger:
			return std::make_shared<PeriodicTrigger>(_effect.getData<Effect::PeriodicTrigger>());
		}
		throw std::runtime_error("CreateEffect: Invalid effect type: " + std::to_string(static_cast<int>(_effect.type)));
	}

	Effects createEffects(const data::Aura::Effects& _effectsDef)
	{
		Effects effects;
		effects.reserve(_effectsDef.size());
		for (auto& def : _effectsDef)
		{
			auto effect = createEffect(def);
			assert(effect);
			effects.emplace_back(std::move(effect));
		}
		return effects;
	}

	void Interface::notify(const Aura& _aura, Event& _event)
	{
		if (_event.getType() == getEventType())
			_notify(_aura, _event);
	}

	void Interface::apply(const Aura& _aura)
	{
	}

	void Interface::remove(const Aura& _aura)
	{
	}

	/*#####
	# ApplyCondition
	#####*/
	ApplyCondition::ApplyCondition(const data::Aura::Effect::ApplyCondition& _data) :
		m_Data(_data)
	{
	}

	void ApplyCondition::_do(const Aura& _aura, bool _apply)
	{
		using Target = data::Aura::Effect::ApplyCondition::Target;
		switch (m_Data.target)
		{
		case Target::unit:
			if (_aura.getTarget().isUnit())
			{
				auto& unitTarget = _aura.getTarget().toUnit();
				unitTarget.applyUnitCondition(fromInt<unit::Condition>(*m_Data.condition), _apply);
			}
			else
				LOG_ERR("AuraEffect: ApplyCondition: Apply unit condition but target is not a unit.");
			break;
		default:
			LOG_ERR("AuraEffect: ApplyCondition: Aura Id: " << *_aura.getAuraInfo().id << " invalid condition");
		}
	}

	void ApplyCondition::apply(const Aura& _aura)
	{
		_do(_aura, true);
	}

	void ApplyCondition::remove(const Aura& _aura)
	{
		_do(_aura, false);
	}

	/*#####
	# LifeLink
	#####*/
	void LifeLink::apply(const Aura& _aura)
	{
		auto& target = _aura.getTarget();
		if (target.isUnit())
			m_LifeLinkContainer.addMember(target.toUnit());
	}

	void LifeLink::remove(const Aura& _aura)
	{
		auto& target = _aura.getTarget();
		if (target.isUnit())
			m_LifeLinkContainer.removeMember(target.toUnit());
	}

	void LifeLink::_notify(const Aura& _aura, Event& _event)
	{
		auto& calcDmgEvent = static_cast<event::CalculateDamage&>(_event);
		calcDmgEvent.getDamageInfo().lifeLinkPool.insert(m_LifeLinkContainer);
	}

	EventType LifeLink::getEventType() const
	{
		return EventType::calculateDamage;
	}

	/*#####
	# ModIncSpellEffectivity
	#####*/
	ModIncSpellEffectivity::ModIncSpellEffectivity(const data::Aura::Effect::ModIncSpellEffectivity& _data) :
		m_Data(_data)
	{
	}

	void ModIncSpellEffectivity::_notify(const Aura& _aura, Event& _event)
	{
		auto& incSpellEvent = static_cast<event::IncomingSpell&>(_event);
		if (incSpellEvent.getSpellId() == *m_Data.spellId &&
			(!_aura.getAuraInfo().flags.contains(AuraFlags::individualCasterStack) ||
			(incSpellEvent.getCaster() && incSpellEvent.getCaster() == _aura.getCaster())))
		{
			incSpellEvent.addBonus(*m_Data.bonus + _aura.getStacks());
		}
	}

	EventType ModIncSpellEffectivity::getEventType() const
	{
		return EventType::incomingSpell;
	}

	/*#####
	# ModifySpell
	#####*/
	ModifySpell::ModifySpell(const data::Aura::Effect::SpellModifier& _data) :
		m_Data(_data)
	{
	}

	void ModifySpell::_notify(const Aura& _aura, Event& _event)
	{
		auto& createSpellEvent = static_cast<event::CreateSpell&>(_event);
		createSpellEvent.addSpellMod(m_Data);
	}

	EventType ModifySpell::getEventType() const
	{
		return EventType::createSpell;
	}

	/*#####
	# ModStat
	#####*/
	ModStat::ModStat(const data::Aura::Effect::ModStat& _modStat, bool _percent) :
		m_Percent(_percent),
		m_Data(_modStat)
	{
	}

	void ModStat::_do(const Aura& _aura, bool _apply)
	{
		using Target = data::Aura::Effect::Target;
		auto stack = std::max(1, _aura.getStacks() + 1);
		auto& target = _aura.getTarget();
		switch (m_Data.target)
		{
		case Target::unit:
		{
			using unit::StatType;
			auto type = fromInt<StatType>(*m_Data.statType);
			if (type != StatType::none)
			{
				auto& statModifier = target.toUnit().statModifier;
				if (m_Percent)
				{
					_apply ? statModifier.applyPercentMod(type, getModifierGroup(), *m_Data.value * stack) :
						statModifier.removePercentMod(type, getModifierGroup(), *m_Data.value * stack);
				}
				else
				{
					_apply ? statModifier.applyFlatMod(type, getModifierGroup(), *m_Data.value * stack) :
						statModifier.removeFlatMod(type, getModifierGroup(), *m_Data.value * stack);
				}
			}
			break;
		}
		case Target::device:
		{
			using device::StatType;
			auto type = fromInt<StatType>(*m_Data.statType);
			if (type != StatType::none)
			{
				auto& statModifier = target.toDevice().statModifier;
				if (m_Percent)
				{
					_apply ? statModifier.applyPercentMod(type, getModifierGroup(), *m_Data.value * stack) :
						statModifier.removePercentMod(type, getModifierGroup(), *m_Data.value * stack);
				}
				else
				{
					_apply ? statModifier.applyFlatMod(type, getModifierGroup(), *m_Data.value * stack) :
						statModifier.removeFlatMod(type, getModifierGroup(), *m_Data.value * stack);
				}
			}
			break;
		}
		}
	}

	void ModStat::apply(const Aura& _aura)
	{
		_do(_aura, true);
	}

	void ModStat::remove(const Aura& _aura)
	{
		_do(_aura, false);
	}

	StatModifierGroup ModStat::getModifierGroup() const
	{
		return m_Data.group;
	}

	/*#####
	# Offender
	#####*/
	Offender::Offender(const data::Aura::Effect::Offender& _data) :
		m_Data(_data)
	{
	}

	void Offender::_notify(const Aura& _aura, Event& _event)
	{
		auto& applyXpEvent = static_cast<event::AppliedXp&>(_event);
		using Reason = event::AppliedXp::Reason;
		switch (applyXpEvent.getReason())
		{
		case Reason::damage:
		case Reason::kill:
		{
			auto caster = _aura.getCaster();
			if (caster && caster->isUnit() && applyXpEvent.getXpAmount() > 0 &&
				(!applyXpEvent.getTrigger() || caster.get() != applyXpEvent.getTrigger()))
			{
				auto& unit = caster->toUnit();
				auto stacks = std::max(1, _aura.getStacks() + 1);
				if (auto level = unit.getLevel())
					level->addXP(applyXpEvent.getXpAmount() * *m_Data.percent * stacks);
			}
			break;
		}
		}
	}

	EventType Offender::getEventType() const
	{
		return EventType::applyXp;
	}

	/*#####
	# PeriodicDealing
	#####*/
	PeriodicDealing::PeriodicDealing(const data::Aura::Effect::PeriodicDealing& _data, bool _damage, bool _percent) :
		m_Data(_data),
		m_Damage(_damage),
		m_Percent(_percent)
	{
	}

	void PeriodicDealing::tick(const Aura& _aura)
	{
		if (!_aura.getTarget().isUnit())
			return;

		auto& unitTarget = _aura.getTarget().toUnit();
		auto stacks = std::max(1, _aura.getStacks() + 1);
		if (m_Damage)
		{
			Stat damage = m_Percent ? unitTarget.getUnitState().stats.getStat(unit::StatType::maxHealth) * *m_Data.amount : *m_Data.amount;
			unitTarget.applyDamage(std::max<Stat>(0, damage * stacks), _aura.getCasterInfo());
		}
		else
		{
			Stat healing = m_Percent ? unitTarget.getUnitState().stats.getStat(unit::StatType::maxHealth) * *m_Data.amount : *m_Data.amount;
			unitTarget.applyHealing(std::max<Stat>(0, healing * stacks), _aura.getCasterInfo());
		}
	}

	bool PeriodicDealing::isPeriodic() const
	{
		return true;
	}

	Time PeriodicDealing::getIntervalTime() const
	{
		return *m_Data.interval;
	}

	/*#####
	# PeriodicTrigger
	#####*/
	PeriodicTrigger::PeriodicTrigger(const data::Aura::Effect::PeriodicTrigger& _data) :
		m_Data(_data)
	{
	}

	void PeriodicTrigger::tick(const Aura& _aura)
	{
		auto& caster = _aura.getTarget();
		if (auto spellDef = data::findData(caster.getGameDependencies().gameData.spells, *m_Data.id))
		{
			for (auto i = 0; i <= _aura.getStacks(); ++i)
			{
				if (*m_Data.perma)
					caster.castTriggered(*spellDef, caster.generateCastableInfo(), caster.getCastableHandle());
				else
					caster.cast(*spellDef, caster.getCastableHandle());
			}
			LOG_DEBUG("AuraEffect: PeriodicTrigger: triggered spell " << *spellDef->id << " " << (_aura.getStacks() + 1) << " times.");
		}
	}

	bool PeriodicTrigger::isPeriodic() const
	{
		return true;
	}

	Time PeriodicTrigger::getIntervalTime() const
	{
		return *m_Data.interval;
	}

	/*#####
	# TriggerSpell
	#####*/
	TriggerSpell::TriggerSpell(const data::Aura::Effect::TriggerSpell& _data) :
		m_Data(_data)
	{
	}

	void TriggerSpell::_notify(const Aura& _aura, Event& _event)
	{
		auto& triggerEvent = static_cast<event::Trigger&>(_event);
		if (triggerEvent.getTriggerType() != m_Data.triggerType)
			return;
		auto& owner = triggerEvent.getOwner();
		if (auto spellDef = data::findData(owner.getGameDependencies().gameData.spells, *m_Data.id))
		{
			auto handle = triggerEvent.getTrigger().getCastableHandle();
			for (auto i = 0; i <= _aura.getStacks(); ++i)
				owner.castTriggered(*spellDef, owner.generateCastableInfo(), handle);
			LOG_DEBUG("AuraEffect: TriggerSpell: triggered spell " << *spellDef->id << " " << (_aura.getStacks() + 1) << " times.");
		}
	}

	EventType TriggerSpell::getEventType() const
	{
		return EventType::trigger;
	}
} // namespace game::aura::effect
