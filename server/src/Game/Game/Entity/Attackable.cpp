#include "stdafx.h"
#include "Attackable.hpp"
#include "Data/Game.hpp"
#include "Data/Spell.hpp"
#include "Data/Unit.hpp"
#include "Map/ColliderMap.hpp"
#include "Spell/TargetFinder.hpp"
#include "Game/Entity/Unit.hpp"
#include "Game/Entity/Participant.hpp"
#include "TargetPreferenceUtil.hpp"

namespace game::unit
{
	Attackable::Attackable(const data::ActiveUnit& _info, const TargetPreference* _preference, Unit& _owner, GameDependencies& _gameDependencies) :
		m_GameDependencies(_gameDependencies),
		m_Info(_info),
		m_Owner(_owner),
		m_TargetPreference(_preference)
	{
		if (auto spell = data::findData(m_GameDependencies.gameData.spells, *m_Info.spellId))
		{
			m_AttackSpell = spell;
			_startAttackTimer();
		}
	}

	void Attackable::_startAttackTimer()
	{
		assert(!m_AttackTimer.isActive());
		m_AttackTimer.start(AttackSpeedBase);
	}

	CastableHandle Attackable::_findTarget() const
	{
		assert(m_AttackSpell);

		if (m_AttackSpell->getFlags().contains(data::Spell::Flags::selfCast))
			return m_Owner.getCastableHandle();

		auto attackArea = getAttackArea();
		auto& targetFinder = m_Owner.getOwner().getTargetFinder();
		auto& targetInfo = m_AttackSpell->getTarget();

		auto isValidTarget = [&targetInfo, &targetFinder, &attackArea, owner = &m_Owner, &spell = m_AttackSpell](const auto& _unit)
		{
			return _unit.isAlive() &&
				targetFinder.hasCorrectTargetFlags(targetInfo, _unit) &&
				_unit.getCollider().overlaps(attackArea) &&
				(!spell->getFlags().contains(data::Spell::Flags::casterNoTarget) || &_unit != owner);
		};

		if (auto globalTarget = m_Owner.getOwner().getGlobalTarget(); globalTarget && isValidTarget(*globalTarget))
			return globalTarget->getCastableHandle();

		std::vector<Unit*> units;
		auto gridEntities = m_GameDependencies.colliderMap.getUniqueFromCells(attackArea);
		units.reserve(std::size(gridEntities));
		for (auto entity : gridEntities)
		{
			if (isValidTarget(entity->getOwner()))
				units.emplace_back(&entity->getOwner());
		}

		Unit* result = nullptr;
		if (m_TargetPreference)
			result = selectUnit(*m_TargetPreference, units, m_Owner.getOwner());
		else if (!std::empty(units))
			result = units[randomInt<std::size_t>(0, std::size(units) - 1)];
		return result ? result->getCastableHandle() : CastableHandle();
	}

	void Attackable::updateAttackable(Time _diff)
	{
		auto attackSpeed = m_Owner.calculateStat(StatType::attackSpeed);
		if (!hasAttackSpell() || attackSpeed <= 0)
			return;
		Time diff(static_cast<int>(_diff.count() * attackSpeed));

		auto processAttack = [&]()
		{
			diff -= std::chrono::duration_cast<Time>(m_AttackTimer.update(diff));
			if (!m_AttackTimer.isActive())
			{
				m_Target = _findTarget();
				if (m_Target.isValid())
				{
					m_Owner.cast(*m_AttackSpell, m_Target);
					_startAttackTimer();
					LOG_DEBUG("Unit guid: " << m_Owner.getGUID() << " attacks.");
					return 0 < diff.count();
				}
			}
			return false;
		};

		/* this is a hack fix to prevent from freezing. Don't know really why it could happen, but we
			had a lot issues here, thus I decided to go this way to keep the server stable.*/
		for (int i = 0; i < 5 && processAttack(); ++i) {}
	}

	void Attackable::restartAttack()
	{
		m_AttackTimer.restart();
	}

	const TargetPreference* Attackable::getTargetPreference() const
	{
		return m_TargetPreference;
	}

	bool Attackable::hasAttackSpell() const
	{
		return m_AttackSpell != nullptr;
	}

	float Attackable::getAttackProgressPercent() const
	{
		return m_AttackTimer.getProgressInPercent();
	}

	CastableHandle Attackable::getTarget() const
	{
		return m_Target;
	}
} // namespace game::unit
