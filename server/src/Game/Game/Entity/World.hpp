#pragma once

#include "Game/Entity/Unit.hpp"
#include "GUIDManager.hpp"

namespace game::unit
{
	class World :
		sl2::NonCopyable
	{
	private:
		using UnitPtr = std::unique_ptr<Unit>;
		std::vector<UnitPtr> m_Units;
		std::vector<UnitPtr> m_New;
		GUIDManager m_GUIDMgr;

	public:
		template <class Unit, typename... Args>
		Unit& createUnit(Args&&... _args)
		{
			static_assert(std::is_base_of<unit::Unit, Unit>::value, "Template class is not a derived class of entity::Unit.");
			auto ptr = std::make_unique<Unit>(std::forward<Args>(_args)...);
			auto& ref = *ptr;
			ref.m_GUID = m_GUIDMgr.getNextGUID();
			m_New.emplace_back(std::move(ptr));
			return ref;
		}

		Unit* findUnit(GUID _guid) const;

		void update(Time _diff, bool _mergeOnly);
	};
} // namespace game::unit
