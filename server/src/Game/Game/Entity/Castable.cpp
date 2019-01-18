#include "stdafx.h"
#include "Castable.hpp"
#include "Spell/TargetFinder.hpp"
#include "Spell/Carrier.hpp"
#include "Spell/SpellManager.hpp"
#include "Spell/Aura.hpp"
#include "Data/Game.hpp"
#include "Spell/AuraEvent.hpp"

namespace game
{
	/*#####
	# AuraHolder
	#####*/
	std::vector<aura::Aura*> AuraHolder::_findIf(std::function<bool(const aura::Aura&)> _predicate) const
	{
		std::vector<aura::Aura*> result;
		for (auto& aura : m_Auras)
		{
			assert(aura);
			if (_predicate(*aura))
				result.push_back(aura.get());
		}
		return result;
	}

	void AuraHolder::notify(aura::Event& _event)
	{
		for (auto& _aura : m_NotifiableAuras)
			_aura->notify(_event);
	}

	void AuraHolder::update(Time _diff)
	{
		for (auto itr = std::begin(m_Auras); itr != std::end(m_Auras);)
		{
			auto& aura = **itr;
			aura.update(_diff);
			if (aura.isErasable())
			{
				if (aura.isNotifiable())
				{
					auto notifyItr = std::find(std::begin(m_NotifiableAuras), std::end(m_NotifiableAuras), &aura);
					assert(notifyItr != std::end(m_NotifiableAuras));
					m_NotifiableAuras.erase(notifyItr);
				}
				auto eraseAura = std::move(*itr);
				itr = m_Auras.erase(itr);
				eraseAura->removeEffects();
			}
			else
				++itr;
		}
	}

	void AuraHolder::add(aura::AuraPtr _aura)
	{
		assert(std::find(std::begin(m_Auras), std::end(m_Auras), _aura) == std::end(m_Auras));

		// we search for auras with same Id or auras with same family (other than 0)
		for (auto oldAura : _findIf(
			[&_aura](const aura::Aura& _other)
			{
				return _aura->getAuraInfo().id == _other.getAuraInfo().id ||
					(_aura->getFamily() != 0 && _aura->getFamily() == _other.getFamily());
			}
		))
		{
			assert(oldAura);
			// we don't want to override or remove auras casted by different caster with individiualCasterStack set.
			if (!oldAura->getAuraInfo().flags.contains(aura::AuraFlags::individualCasterStack) ||
				oldAura->getCaster() == _aura->getCaster())
			{
				if (oldAura->getAuraInfo().id == _aura->getAuraInfo().id)
				{
					_aura->setStacks(oldAura->getStacks() + 1);
					if (oldAura->getAuraInfo().flags.contains(aura::AuraFlags::accumulateDuration))
						_aura->setDuration(_aura->getDuration() + oldAura->getDuration());
				}
				_remove(*oldAura);
			}
		}

		if (_aura->isNotifiable())
			m_NotifiableAuras.push_back(_aura.get());

		_aura->applyEffects();
		m_Auras.push_back(std::move(_aura));
	}

	void AuraHolder::_remove(aura::Aura& _aura)
	{
		assert(std::end(m_Auras) != std::find_if(std::begin(m_Auras), std::end(m_Auras),
			[&_aura](const aura::AuraPtr& _other) { return &_aura == _other.get();}
		));
		_aura.setErasable();
	}

	void AuraHolder::clear()
	{
		for (auto& aura : m_Auras)
			aura->removeEffects();
		m_Auras.clear();
		m_NotifiableAuras.clear();
	}

	std::vector<const aura::Aura*> AuraHolder::findAuras(std::function<bool(const aura::Aura&)> _predicate) const
	{
		auto result = _findIf(_predicate);
		return reinterpret_cast<std::vector<const aura::Aura*>&>(result);
	}

	/*#####
	# Castable
	#####*/
	Castable::Castable(const CastableInitializer& _initializer) :
		m_MasterHandle(*this),
		m_GameDependencies(_initializer.gameDependencies),
		m_TargetFinder(&_initializer.targetFinder)
	{
	}

	const spell::TargetFinder& Castable::getTargetFinder() const
	{
		assert(m_TargetFinder);
		return *m_TargetFinder;
	}

	GameDependencies& Castable::getGameDependencies() const
	{
		return m_GameDependencies;
	}

	CastableHandle Castable::getCastableHandle() const
	{
		return m_MasterHandle.get();
	}

	bool Castable::isAffectedBySpell() const
	{
		return m_IncomingSpellCounter > 0 || m_OutgoingSpellCounter > 0;
	}
	
	bool Castable::canCast() const
	{
		return true;
	}

	bool Castable::cast(data::Spell _spellInfo, AbsPosition _pos)
	{
		return _cast(std::move(_spellInfo), generateCastableInfo(), _pos, false);
	}

	bool Castable::cast(data::Spell _spellInfo, CastableHandle _target)
	{
		return _cast(std::move(_spellInfo), generateCastableInfo(), _target, false);
	}

	bool Castable::castTriggered(data::Spell _spellInfo, CastableSnapshot _snapshot, AbsPosition _pos)
	{
		return _cast(std::move(_spellInfo), _snapshot, _pos, true);
	}
	
	bool Castable::castTriggered(data::Spell _spellInfo, CastableSnapshot _snapshot, CastableHandle _target)
	{
		return _cast(std::move(_spellInfo), _snapshot, _target, true);
	}

	void Castable::updateCastable(Time _diff)
	{
		m_AuraHolder.update(_diff);
	}
	
	void Castable::addAura(aura::AuraPtr _aura)
	{
		assert(_aura && &_aura->getTarget() == this);
		m_AuraHolder.add(std::move(_aura));
	}

	void Castable::notifyAuras(aura::Event& _event)
	{
		m_AuraHolder.notify(_event);
	}
	
	std::vector<const aura::Aura*> Castable::findAuras(std::function<bool(const aura::Aura&)> _predicate) const
	{
		return m_AuraHolder.findAuras(_predicate);
	}

	IDs Castable::gatherAuraVisualIds() const
	{
		auto auras = findAuras([](const aura::Aura& _aura) {
			return _aura.getAuraInfo().visualId != 0;
		});

		if (!auras.empty())
		{
			IDs result;
			result.reserve(auras.size());
			std::transform(std::begin(auras), std::end(auras), std::back_inserter(result), [](const aura::Aura* _aura){
				return *_aura->getAuraInfo().visualId;
			});
			return result;
		}
		return {};
	}

	void Castable::clearAuras()
	{
		m_AuraHolder.clear();
	}

	void Castable::setTargetFinder(const spell::TargetFinder& _targetFinder)
	{
		m_TargetFinder = &_targetFinder;
	}

	CastableSnapshot Castable::generateCastableInfo() const
	{
		CastableSnapshot info;
		setupCastableSnapshot(info);
		return info;
	}
	
	void Castable::setupCastableSnapshot(CastableSnapshot& _info) const
	{
		_info.owner = getCastableHandle();
		aura::event::CreateSpell event(const_cast<Castable&>(*this));	// ugly hack, but I am lazy...
		const_cast<Castable&>(*this).notifyAuras(event);
		_info.spellModifiers = event.getSpellMods();
	}
} // namespace game::entity
