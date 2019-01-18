#pragma once

#include "Others.hpp"
#include "Core/SharedDefines.hpp"
#include "Core/StatContainer.hpp"
#include "PropertyDefines.hpp"

namespace game::data
{
	struct Device
	{
	public:
		using StatType = device::StatType;
		using XpBoostModifier = XpBoostModifier<StatType>;
		using Stats = StatContainer<StatType>;

	private:
		Stats m_Stats;
		XpBoostModifier m_XpBoostModifier;
		Stats& _validateStats(Stats& _stats) const;

	public:
		prop::Property<ID> id{ prop::Id };
		prop::Property<std::string> name{ prop::Name };
		prop::Property<std::string> description{ "", "description" };
		prop::Property<bool> pickable{ "pickable", true };

		void setStats(Stats _stats);
		const Stats& getStats() const;

		void setXpBoostModifier(XpBoostModifier _mods);
		const XpBoostModifier& getXpBoostModifier() const;

		virtual std::string_view getTypeName() const = 0;
	};
} // namespace game::data
