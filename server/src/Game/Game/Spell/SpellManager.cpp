#include "stdafx.h"
#include "SpellManager.hpp"

namespace game::spell
{
	bool SpellManager::addSpell(SpellPtr _spell)
	{
		if (_spell && _spell->isValid())
		{
			if (m_IsInUpdate && _spell->isInstant() && _spell->isTriggered())
				_spell->exec();
			else
				m_New.emplace_back(spell::carrier::createCarrier(std::move(_spell)));
			return true;
		}
		return false;
	}

	void SpellManager::update(Time _diff)
	{
		m_IsInUpdate = true;

		if (!std::empty(m_New))
		{
			m_Carriers.insert(std::end(m_Carriers), std::make_move_iterator(std::begin(m_New)), std::make_move_iterator(std::end(m_New)));
			m_New.clear();
		}

		m_Carriers.erase(std::remove_if(std::begin(m_Carriers), std::end(m_Carriers),
			[_diff](auto& _carrier)
			{
				_carrier->update(_diff);
				return _carrier->hasFinished();
			}
		), std::end(m_Carriers));

		m_IsInUpdate = false;
	}
} // namespace game::spell
