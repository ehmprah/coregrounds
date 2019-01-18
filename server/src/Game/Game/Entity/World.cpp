#include "stdafx.h"
#include "World.hpp"

namespace game::unit
{
	void _mergeNewUnits(std::vector<std::unique_ptr<Unit>>& _units, std::vector<std::unique_ptr<Unit>>& _newUnits)
	{
		if (!_newUnits.empty())
		{
			for (auto& unit : _newUnits)
				unit->startPlay();
			_units.insert(std::end(_units), std::make_move_iterator(std::begin(_newUnits)), std::make_move_iterator(std::end(_newUnits)));
			_newUnits.clear();
		}
	}

	Unit* World::findUnit(GUID _guid) const
	{
		auto itr = std::find_if(std::begin(m_Units), std::end(m_Units), [_guid](const UnitPtr& _unit) {
			return _unit->getGUID() == _guid;
		});
		return itr != std::end(m_Units) ? itr->get() : nullptr;
	}

	void World::update(Time _diff, bool _mergeOnly)
	{
		if (!_mergeOnly)
		{
			for (auto& unit : m_Units)
				unit->update(_diff);

			// finalize tick and remove all dead units
			for (auto itr = std::begin(m_Units); itr != std::end(m_Units);)
			{
				assert(*itr);
				auto& unit = **itr;
				unit.finalizeTick();
				if (unit.isErasable())
					itr = m_Units.erase(itr);
				else
					++itr;
			}
		}

		_mergeNewUnits(m_Units, m_New);
	}
} // namespace game::unit
