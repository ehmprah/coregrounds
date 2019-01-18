#include "stdafx.h"
#include "Aura.hpp"
#include "Data/Others.hpp"
#include "Game/Entity/Castable.hpp"

namespace game::aura
{
	AuraPtr createAura(const data::Aura& _auraInfo, Castable& _target, const CastableSnapshot& _casterInfo)
	{
		return createAura(_auraInfo, effect::createEffects(_auraInfo.getEffects()), _target, _casterInfo);
	}

	AuraPtr createAura(const data::Aura& _auraInfo, effect::Effects _effects, Castable& _target, const CastableSnapshot& _casterInfo)
	{
		return std::make_unique<Aura>(_target, _casterInfo, _auraInfo, _effects);
	}

	Aura::Aura(Castable& _target, const CastableSnapshot& _casterInfo, const data::Aura& _auraInfo, effect::Effects _effects) :
		m_AuraInfo(_auraInfo),
		m_Effects(std::move(_effects)),
		m_Target(_target),
		m_CasterInfo(_casterInfo)
	{
		_startDurationIfNecessary();
		_calculateEventTypes();
		_findPeriodicEffects();
	}

	void Aura::_calculateEventTypes()
	{
		for (auto& effect : m_Effects)
			m_EventTypes |= static_cast<std::uint32_t>(effect->getEventType());
	}

	void Aura::_doEffects(bool _apply)
	{
		for (auto& effect : m_Effects)
			_apply ? effect->apply(*this) : effect->remove(*this);
	}

	void Aura::_startDurationIfNecessary()
	{
		if (isTimed())
			m_Duration.start(*m_AuraInfo.duration);
	}

	void Aura::_findPeriodicEffects()
	{
		for (auto& effect : m_Effects)
		{
			assert(effect);
			if (effect->isPeriodic())
				m_PeriodicEffects.emplace_back(effect.get(), effect->getIntervalTime());
		}
	}

	void Aura::notify(Event& _event)
	{
		for (auto& effect : m_Effects)
			effect->notify(*this, _event);
	}

	std::uint32_t Aura::getEventTypes() const
	{
		return m_EventTypes;
	}

	bool Aura::isNotifiable() const
	{
		return getEventTypes() != 0;
	}

	int Aura::getFamily() const
	{
		return *m_AuraInfo.family;
	}

	bool Aura::isStackable() const
	{
		return getMaxStacks() > 0;
	}

	int Aura::getMaxStacks() const
	{
		return *m_AuraInfo.maxStacks;
	}

	int Aura::getStacks() const
	{
		return m_Stacks;
	}

	void Aura::setStacks(int _stacks)
	{
		if (isStackable())
			m_Stacks = std::clamp(_stacks, 0, getMaxStacks());
	}

	Time Aura::getDuration() const
	{
		return m_Duration.getTimerValueInMsec();
	}

	void Aura::setDuration(Time _time)
	{
		m_Duration.start(_time);
	}

	const data::Aura& Aura::getAuraInfo() const
	{
		return m_AuraInfo;
	}

	Castable& Aura::getTarget() const
	{
		return m_Target;
	}

	CastableHandle Aura::getCaster() const
	{
		return m_CasterInfo.owner;
	}

	const CastableSnapshot& Aura::getCasterInfo() const
	{
		return m_CasterInfo;
	}

	void Aura::update(Time _diff)
	{
		for (auto& [effect, timer] : m_PeriodicEffects)
		{
			assert(effect);
			timer.update(_diff);
			if (timer.getState() == Timer::State::justFinished)
			{
				effect->tick(*this);
				timer.start(effect->getIntervalTime());
			}
		}

		if (isTimed())
		{
			m_Duration.update(_diff);
			if (m_Duration.getState() == Timer::State::justFinished)
				setErasable();
		}
	}

	bool Aura::isTimed() const
	{
		return Time::zero() < m_AuraInfo.duration;
	}

	bool Aura::isPersistent() const
	{
		return !isTimed();
	}

	bool Aura::isExpired() const
	{
		return isTimed() && m_Duration.getState() == Timer::State::justFinished;
	}

	bool Aura::isErasable() const
	{
		return m_IsErasable;
	}

	void Aura::setErasable()
	{
		m_IsErasable = true;
	}

	void Aura::applyEffects()
	{
		_doEffects(true);
	}

	void Aura::removeEffects()
	{
		_doEffects(false);
		m_PeriodicEffects.clear();
		m_Effects.clear();
	}

	const effect::Effects& Aura::getEffects() const
	{
		return m_Effects;
	}
} // namespace game::aura
