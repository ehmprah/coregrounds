#include "stdafx.h"
#include "LifeLink.hpp"
#include "Game/Entity/Unit.hpp"

namespace game::unit
{
	const std::vector<Unit*> LifeLinkContainer::getMembers() const
	{
		return m_Members;
	}

	void LifeLinkContainer::addMember(Unit& _unit)
	{
		assert(std::find(std::begin(m_Members), std::end(m_Members), &_unit) == std::end(m_Members));
		m_Members.emplace_back(&_unit);
	}

	void LifeLinkContainer::_killAllUnits(const CastableSnapshot& _dealer, const Unit& _originalReceiver)
	{
		auto tmpMembers = std::move(m_Members);
		for (auto unit : tmpMembers)
		{
			if (unit == &_originalReceiver)
				m_Members.emplace_back(unit);
			else
				unit->kill(_dealer);
		}
	}

	void LifeLinkContainer::removeMember(Unit& _unit)
	{
		auto itr = std::find(std::begin(m_Members), std::end(m_Members), &_unit);
		assert(itr != std::end(m_Members));
		m_Members.erase(itr);
	}

	/*#####
	# LifeLinkPool
	#####*/
	void LifeLinkPool::insert(const LifeLinkContainer& _container)
	{
		m_Members.reserve(m_Members.size() + _container.getMembers().size());
		for (auto unit : _container.getMembers())
		{
			auto itr = std::lower_bound(std::begin(m_Members), std::end(m_Members), unit);
			if (itr == std::end(m_Members) || *itr != unit)
			{
				m_Members.insert(itr, unit);
				m_PoolHP += unit->getUnitState().health;
			}
		}
	}

	Stat LifeLinkPool::getTotalHP() const
	{
		return m_PoolHP;
	}

	std::vector<std::pair<Unit*, Stat>> LifeLinkPool::calculateSharedDamage(Stat _damage) const
	{
		std::vector<std::pair<Unit*, Stat>> result;
		result.reserve(m_Members.size());
		if (_damage >= getTotalHP())
		{
			std::transform(std::begin(m_Members), std::end(m_Members), std::end(result),
				[](auto _unit) { return std::make_pair(_unit, _unit->getUnitState().health); }
			);
			return result;
		}

		// sort by current hp (ascending order)
		auto sortedPool = m_Members;
		std::sort(std::begin(sortedPool), std::end(sortedPool), [](const Unit* _lhs, const Unit* _rhs) {
			return _lhs->getUnitState().health < _rhs->getUnitState().health;
		});

		// share damage with every member in pool
		auto poolSize = sortedPool.size();
		for (auto itr = std::begin(sortedPool); itr != std::end(sortedPool) && _damage > 0; ++itr, --poolSize)
		{
			auto damageForEach = _damage / poolSize;
			assert(*itr);
			auto& unit = **itr;

			static const Stat minHP = std::numeric_limits<Stat>::min();
			if (unit.getUnitState().health <= minHP)
				continue;

			auto damageToStayAlive = unit.getUnitState().health - minHP;
			auto damageToUnit = damageToStayAlive < damageForEach ? damageToStayAlive : damageForEach;
			_damage -= damageToUnit;
			result.push_back(std::make_pair(&unit, damageToUnit));
		}

		// if there is dmg left, kill unit after unit until every dmg is shared
		for (auto itr = std::begin(result); itr != std::end(result) && _damage > 0; ++itr)
		{
			auto& unit = *itr->first;
			_damage += itr->second - unit.getUnitState().health;
			itr->second = unit.getUnitState().health;
		}
		return result;
	}
} // namespace game::unit
