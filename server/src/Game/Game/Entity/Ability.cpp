#include "stdafx.h"
#include "Ability.hpp"
#include "Data/Ability.hpp"
#include "Data/Game.hpp"
#include "Game/Spell/TargetFinder.hpp"
#include "Game/Entity/Unit.hpp"
#include "Core/HelperFunctions.hpp"

namespace game::device
{
	/*#####
	# Ability
	#####*/
	const data::Spell& _lookupSpellData(const data::Game& _gameData, ID _abilityId, ID _spellId)
	{
		if (auto spell = findData(_gameData.spells, _spellId))
			return *spell;
		throw std::runtime_error("Ability id:" + std::to_string(_abilityId) + " Invalid spellId: " + std::to_string(_spellId));
	}

	Ability::Ability(const AbilityInitializer& _initializer) :
		super(_initializer.deviceInit),
		m_AbilityData(_initializer.abilityData),
		m_SpellData(_lookupSpellData(getGameDependencies().gameData, *_initializer.abilityData.id, *_initializer.abilityData.spellId))
	{
	}

	int Ability::getUses() const
	{
		assert(m_Uses >= 0);
		return m_Uses;
	}

	bool Ability::_cast(const AbsPosition& _pos)
	{
		if (getAbilityData().flags.contains(AbilityFlag::requireUnitTarget))
		{
			if (auto targets = getTargetFinder().findUnits(m_SpellData.getTarget(), _pos, data::Shape()); !std::empty(targets))
			{
				auto target = targets[randomInt<std::size_t>(0, std::size(targets) - 1)];
				return cast(m_SpellData, target->getCastableHandle());
			}
			return false;
		}
		return cast(m_SpellData, _pos);
	}

	void Ability::activate(const AbsPosition& _pos)
	{
		if (!m_Cooldown.isActive() && _cast(_pos))
		{
			++m_Uses;
			m_Cooldown.start(*getAbilityData().cooldown);
		}
	}

	void Ability::update(Time _diff)
	{
		updateCastable(_diff);

		updateTimerRelatedToStat(m_Cooldown, _diff, *getAbilityData().cooldown, getStat(*this, StatType::cooldownSpeed));
	}

	void Ability::serialize(protobuf::out::Match_Game_Ability& _msg) const
	{
		_msg.set_id(setHighestBit(*getAbilityData().id));
		_msg.set_cooldown(setHighestBit(static_cast<std::uint32_t>(std::chrono::duration_cast<Time>(m_Cooldown.getTimerValue()).count() / 100) |
			(static_cast<std::uint32_t>(m_Cooldown.getProgressInPercent()) << 16)));

#ifndef NDEBUG
		for (std::size_t i = static_cast<std::size_t>(StatType::none) + 1; i < static_cast<std::size_t>(StatType::max); ++i)
			_msg.add_stats(getStat(*this, static_cast<StatType>(i)));
#endif

		auto& visIdMsg = *_msg.mutable_auravisualids();
		visIdMsg.set_isset(true);
		for (auto id : gatherAuraVisualIds())
			visIdMsg.add_ids(id);
	}

	const data::Ability& Ability::getAbilityData() const
	{
		return m_AbilityData;
	}

	void Ability::derivedStartPlay()
	{
		m_Cooldown.start(*getAbilityData().cooldown);
	}
} // namespace game::device
