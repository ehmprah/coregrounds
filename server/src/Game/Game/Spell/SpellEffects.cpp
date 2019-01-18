#include "stdafx.h"
#include "Data/Game.hpp"
#include "SpellEffects.hpp"
#include "Aura.hpp"
#include "Spell.hpp"
#include "SpellManager.hpp"
#include "Game/Entity/Unit.hpp"
#include "Game/Entity/Participant.hpp"
#include "AuraEvent.hpp"

namespace game::spell::effect
{
	EffectPtr createEffect(const data::Spell::Effect& _effect)
	{
		using Type = data::Spell::Effect::Type;
		switch (_effect.type)
		{
		case Type::applyAura:
			return std::make_unique<ApplyAura>(_effect);
		case Type::instantKill:
			return std::make_unique<InstantKill>(_effect);
		case Type::damage:
			return std::make_unique<DealDamage>(_effect);
		case Type::healing:
			return std::make_unique<DealHealing>(_effect);
		case Type::healthAsDamage:
			return std::make_unique<DealHealthAsDamage>(_effect);
		case Type::triggerSpell:
			return std::make_unique<TriggerSpell>(_effect);
		case Type::spawnMinion:
			return std::make_unique<SpawnMinion>(_effect);
		case Type::spawnTrigger:
			return std::make_unique<SpawnTrigger>(_effect);
		case Type::changeOwner:
			return std::make_unique<ChangeOwner>(_effect);
		default:
			throw std::runtime_error("CreateEffect: Invalid effect type: " + std::to_string(static_cast<int>(_effect.type)));
		}
		return nullptr;
	}

	Stat _calculateSpellEffectivity(const Spell& _spell, Castable& _target)
	{
		auto hops = _spell.getExecutions();
		aura::event::IncomingSpell event(_target, _spell.getCaster().getCastableHandle(), *_spell.getSpellInfo().id);
		_target.notifyAuras(event);
		return std::pow(*_spell.getSpellInfo().effectivity + 1, std::max(0, hops + event.getBonus()));
	}

	Interface::Interface(const data::Spell::Effect& _effectInfo) :
		m_EffectInfo(_effectInfo)
	{
	}

	const data::Spell::Effect& Interface::getInfo() const
	{
		return m_EffectInfo;
	}

	void Interface::exec(const Spell& _spell, Targets& _targets)
	{
		for (auto target : _targets)
		{
			assert(target);
			_execOnTarget(_spell, *target);
		}
	}

	InstantKill::InstantKill(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::instantKill);
	}

	void InstantKill::_execOnTarget(const Spell& _spell, Castable& _target)
	{
		if (_target.isUnit())
		{
			_target.toUnit().kill(_spell.getCasterInfo());
		}
		else
			LOG_ERR("SpellEffects::InstantKill: spell id: " << *_spell.getSpellInfo().id << " detected non-unit target.");
	}

	DealDamage::DealDamage(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::damage);
	}

	void DealDamage::_execOnTarget(const Spell& _spell, Castable& _target)
	{
		if (_target.isUnit())
		{
			auto baseDmg = *getInfo().getData<data::Spell::Effect::Value>().value + _spell.getCasterInfo().unitStats.getStat(unit::StatType::damage);
			auto effectivity = _calculateSpellEffectivity(_spell, _target);
			auto value = std::max<Stat>(baseDmg * effectivity, 0);
			_target.toUnit().applyDamage(value, _spell.getCasterInfo());
		}
		else
			LOG_ERR("SpellEffects::DealDamage: spell id: " << *_spell.getSpellInfo().id << " detected non-unit target.");
	}

	DealHealing::DealHealing(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::healing);
	}

	void DealHealing::_execOnTarget(const Spell& _spell, Castable& _target)
	{
		if (_target.isUnit())
		{
			auto baseHealing = *getInfo().getData<data::Spell::Effect::Value>().value + _spell.getCasterInfo().unitStats.getStat(unit::StatType::healing);
			auto effectivity = _calculateSpellEffectivity(_spell, _target);
			auto value = std::max<Stat>(baseHealing * effectivity, 0);
			_target.toUnit().applyHealing(value, _spell.getCasterInfo());
		}
		else
			LOG_ERR("SpellEffects::DealHealing: spell id: " << *_spell.getSpellInfo().id << " detected non-unit target.");
	}

	DealHealthAsDamage::DealHealthAsDamage(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::healthAsDamage);
	}

	void DealHealthAsDamage::_execOnTarget(const Spell& _spell, Castable& _target)
	{
		if (_target.isUnit())
		{
			auto dmg = std::max<Stat>(0, _spell.getCasterInfo().health * *getInfo().getData<data::Spell::Effect::Percentage>().value);
			auto effectivity = _calculateSpellEffectivity(_spell, _target);
			dmg = std::max<Stat>(dmg * effectivity, 0);
			_target.toUnit().applyDamage(dmg, _spell.getCasterInfo());
		}
		else
			LOG_ERR("SpellEffects::DealHealthAsDamage: spell id: " << *_spell.getSpellInfo().id << " detected non-unit target.");
	}

	ApplyAura::ApplyAura(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::applyAura);
	}

	void ApplyAura::exec(const Spell& _spell, Targets& _targets)
	{
		if (_targets.empty())
			return;

		auto& gameDependencies = _spell.getGameDependencies();
		auto id = *getInfo().getData<data::Spell::Effect::Id>().value;
		if (auto auraDef = data::findData(gameDependencies.gameData.auras, id))
		{
			auto effects = aura::effect::createEffects(auraDef->getEffects());
			for (auto target : _targets)
				target->addAura(aura::createAura(*auraDef, effects, *target, _spell.getCasterInfo()));
		}
		else
			LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - ApplyAura: Invalid aura id: " << id);
	}

	TriggerSpell::TriggerSpell(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::triggerSpell);
	}

	void TriggerSpell::_trigger(const Spell& _spell, Castable& _target, data::Spell _spellData)
	{
		auto triggeredId = *_spellData.id;
		if (!_spell.getGameDependencies().spellManager.addSpell(createSpell(std::move(_spellData), _spell.getGameDependencies(),
			_spell.getTargetFinder(), _spell.getCaster(), _spell.getCasterInfo(), _target.getCastableHandle(), true)))
		{
			LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - Trigger Spell: Failed to trigger spell id: " << triggeredId << " Spell is not valid.");
		}
	}

	void TriggerSpell::exec(const Spell& _spell, Targets& _targets)
	{
		auto& spells = _spell.getGameDependencies().gameData.spells;
		auto& triggerData = getInfo().getData<data::Spell::Effect::TriggerSpell>();
		if (auto spellPtr = data::findData(spells, *triggerData.id))
		{
			auto spellData = *spellPtr;
			for (auto& mod : _spell.getCasterInfo().spellModifiers)
			{
				if (mod->spellId == spellData.id)
					spellData.modify(*mod);
			}

			if (*triggerData.forward)
			{
				auto triggeredId = *spellData.id;
				if (_spell.getGameDependencies().spellManager.addSpell(createSpell(std::move(spellData), _spell.getGameDependencies(),
					_spell.getTargetFinder(), _spell.getCaster(), _spell.getCasterInfo(), _spell.copyDestination(), true)))
				{
					LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - Trigger Spell: Failed to trigger spell id: " << triggeredId << " Spell is not valid.");
				}
			}
			else
			{
				for (auto target : _targets)
				{
					assert(target);
					_trigger(_spell, *target, spellData);
				}
			}
		}
		else
			LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - Trigger Spell: Invalid spell id: " << *triggerData.id);
	}

	SpawnMinion::SpawnMinion(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::spawnMinion);
	}

	void SpawnMinion::exec(const Spell& _spell, Targets& _targets)
	{
		auto& casterInfo = _spell.getCasterInfo();
		if (casterInfo.participant)
		{
			if (auto pos = _spell.getDestinationPosition())
				casterInfo.participant->createMinion(*getInfo().getData<data::Spell::Effect::Id>().value, *pos, casterInfo.targetPreference, casterInfo.level);
			else
				LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - SpawnMinion: position information is missing.");
		}
		else
			LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - SpawnMinion: Missing participant.");
	}

	SpawnTrigger::SpawnTrigger(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::spawnTrigger);
	}

	void SpawnTrigger::exec(const Spell& _spell, Targets& _targets)
	{
		if (_spell.getCasterInfo().participant)
		{
			if (auto pos = _spell.getDestinationPosition())
				_spell.getCasterInfo().participant->createTrigger(*getInfo().getData<data::Spell::Effect::Id>().value, *pos);
			else
				LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - SpawnTrigger: position information is missing.");
		}
		else
			LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - SpawnTrigger: Missing participant.");
	}

	ChangeOwner::ChangeOwner(const data::Spell::Effect& _effect) :
		super(_effect)
	{
		assert(getInfo().type == EffectType::changeOwner);
	}

	void ChangeOwner::_execOnTarget(const Spell& _spell, Castable& _target)
	{
		if (_target.isUnit())
		{
			if (_spell.getCasterInfo().participant)
			{
				auto& unit = _target.toUnit();
				if (unit::isAlive(unit) && unit::isInWorld(unit))
				{
					auto& participant = *_spell.getCasterInfo().participant;
					unit.setOwner(unit::isHostile(unit, participant) ? participant : participant.getOpponent());
				}
			}
			else
				LOG_ERR("Spell id: " << *_spell.getSpellInfo().id << " SpellEffect - ChangeOwner: Failed to change owner. Caster did not provide any owner information.");
		}
		else
			LOG_ERR("SpellEffects::ChangeOwner: spell id: " << *_spell.getSpellInfo().id << " detected non-unit target.");
	}
} // namespace game
