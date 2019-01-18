#pragma once

#include "Core/CastableHandle.hpp"
#include "Core/StatContainer.hpp"
#include "Core/EntityFwd.hpp"
#include "Core/SharedDefines.hpp"

namespace game
{
namespace data
{
	class UnitModifiers;
	struct SpellModifier;
} // namespace data

	struct CastableSnapshot
	{
		Stat health = 0;
		StatContainer<unit::StatType> unitStats;

		CastableHandle owner;
		Participant* participant					= nullptr;
		Level* level								= nullptr;
		const TargetPreference* targetPreference	= nullptr;
		const data::UnitModifiers* unitModifiers	= nullptr;
		std::vector<const data::SpellModifier*> spellModifiers;
	};
} // namespace game
