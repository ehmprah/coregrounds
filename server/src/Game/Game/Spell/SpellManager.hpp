#pragma once

#include "Carrier.hpp"

namespace game::spell
{
	class SpellManager :
		sl2::NonCopyable
	{
	public:
		void update(Time _diff);
		bool addSpell(SpellPtr _spell);

	private:
		std::vector<carrier::CarrierPtr> m_Carriers;
		std::vector<carrier::CarrierPtr> m_New;
		bool m_IsInUpdate = false;

		bool _handleSpell(SpellPtr _spell);
	};
} // namespace game::spell
