#include "stdafx.h"
#include "Spell.hpp"
#include "Data/Spell.hpp"
#include "SpellEffects.hpp"
#include "Game/Entity/Unit.hpp"
#include "TargetFinder.hpp"
#include "Core/HelperFunctions.hpp"
#include "Game/Entity/Castable.hpp"
#include "VisualCollector.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"

namespace game::spell
{
	SpellPtr _createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, DestinationPtr _target, bool _triggered)
	{
		for (auto& mod : _casterInfo.spellModifiers)
		{
			if (mod->spellId == _spellInfo.id)
				_spellInfo.modify(*mod);
		}

		effect::Effects effects;
		effects.reserve(_spellInfo.getEffects().size());
		for (auto& effect : _spellInfo.getEffects())
		{
			auto spellEff = effect::createEffect(effect);
			assert(spellEff);
			effects.push_back(std::move(spellEff));
		}
		effects.shrink_to_fit();
		return std::make_unique<Spell>(std::move(_spellInfo), _gameDependencies, _targetFinder, _caster, _casterInfo, std::move(effects), std::move(_target), _triggered);
	}

	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, AbsPosition _pos, bool _triggered)
	{
		return _createSpell(std::move(_spellInfo), _gameDependencies, _targetFinder, _caster, _casterInfo, createDestination<PositionDestination>(_targetFinder, _pos), _triggered);
	}

	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, CastableHandle _target, bool _triggered)
	{
		return _createSpell(std::move(_spellInfo), _gameDependencies, _targetFinder, _caster, _casterInfo, createDestination<CastableDestination>(_targetFinder, _target), _triggered);
	}

	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, DestinationPtr _target, bool _triggered)
	{
		return _createSpell(std::move(_spellInfo), _gameDependencies, _targetFinder, _caster, _casterInfo, std::move(_target), _triggered);
	}

	/*#####
	# Spell
	#####*/
	Spell::Spell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, effect::Effects _effects, DestinationPtr _target, bool _triggered) :
		m_SpellInfo(std::move(_spellInfo)),
		m_Effects(std::move(_effects)),
		m_Caster(_caster),
		m_Destination(std::move(_target)),
		m_CasterInfo(std::move(_casterInfo)),
		m_TargetFinder(_targetFinder),
		m_GameDependencies(_gameDependencies),
		m_Triggered(_triggered)
	{
		++m_Caster.m_OutgoingSpellCounter;
		assert(m_Destination);
		_setupTarget();
		_setupArea();
	}

	Spell::~Spell()
	{
		--m_Caster.m_OutgoingSpellCounter;
	}

	GameDependencies& Spell::getGameDependencies() const
	{
		return m_GameDependencies;
	}

	const TargetFinder& Spell::getTargetFinder() const
	{
		return m_TargetFinder;
	}

	void Spell::_setupArea()
	{
		m_Area = getSpellInfo().getShape();
		if (getSpellInfo().getFlags().contains(SpellFlags::unitRangeAsAreaSize))
		{
			auto range = getCasterInfo().unitStats.getStat(unit::StatType::attackRange);
			assert(range >= 0);
			if (m_Area.hasData<data::Shape::Circle>())
			{
				data::Shape::Circle data;
				data.radius = range;
				m_Area.setData(data);
			}
			else if (m_Area.hasData<data::Shape::Rect>())
			{
				data::Shape::Rect data;
				data.width = range;
				data.height = range;
				m_Area.setData(data);
			}
		}
	}

	void Spell::_setupTarget()
	{
		if (getSpellInfo().getFlags().contains(SpellFlags::selfCast))
			m_Destination = createDestination<SelfDestination>(m_TargetFinder, m_Caster);
	}

	void Spell::_doEffects(Targets& _targets) const
	{
		for (auto& effect : m_Effects)
			effect->exec(*this, _targets);
	}

	const CastableSnapshot& Spell::getCasterInfo() const
	{
		return m_CasterInfo;
	}

	void Spell::exec()
	{
		if (!isValid())
		{
			LOG_ERR("SpellID: " << *getSpellInfo().id << " Interrupt cast.");
			return;
		}
		assert(m_Destination);
		auto pos = getDestinationPosition();
		auto targets = m_Destination->findTargets(*this);
		if (getSpellInfo().getFlags().contains(SpellFlags::casterNoTarget))
		{
			if (auto itr = std::find(std::begin(targets), std::end(targets), &m_Caster); itr != std::end(targets))
			{
				std::swap(*itr, targets.back());
				targets.pop_back();
			}
		}
		_doEffects(targets);
		++m_Executions;

		// handle visual stuff
		if (*getSpellInfo().visualId > 0 && pos)
		{
			if (m_CasterInfo.participant)
			{
				m_GameDependencies.gamePhase.getVisualCollector().addVisual(m_CasterInfo.participant->getIndex(), { *getSpellInfo().visualId, *pos, getShape() });
			}
		}
	}

	int Spell::getExecutions() const
	{
		return m_Executions;
	}

	bool Spell::isTriggered() const
	{
		return m_Triggered;
	}

	const game::data::Spell& Spell::getSpellInfo() const
	{
		return m_SpellInfo;
	}

	Castable& Spell::getCaster() const
	{
		return m_Caster;
	}

	std::optional<game::AbsPosition> Spell::getDestinationPosition() const
	{
		assert(m_Destination);
		if (auto pos = m_Destination->getPosition())
			return getSpellInfo().getFlags().contains(SpellFlags::tileAligned) ? makeTileCenterPosition(*pos) : *pos;
		return {};
	}

	CastableHandle Spell::getDestinationEntity() const
	{
		assert(m_Destination);
		return m_Destination->getDestinationEntity();
	}

	bool Spell::changeDestination(const AbsShape& _shape, bool _uniqueCheck)
	{
		assert(m_Destination);
		return m_Destination->nextDestination(getSpellInfo().getTarget(), _shape, _uniqueCheck);
	}

	DestinationPtr Spell::copyDestination() const
	{
		assert(m_Destination);
		return m_Destination->copy();
	}

	const data::Shape& Spell::getShape() const
	{
		return m_Area;
	}

	bool Spell::isValid() const
	{
		if (!m_Destination)
			return false;
		return true;
	}

	bool Spell::isDelayed() const
	{
		return !isInstant();
	}

	bool Spell::isInstant() const
	{
		return m_SpellInfo.delay->count() <= 0;
	}

	bool Spell::isProjectile() const
	{
		return getSpellInfo().projectileId > 0;
	}
} // namespace game::spell
