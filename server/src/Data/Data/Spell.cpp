#include "Spell.hpp"
#include "DataStringHelper.hpp"
#include "Core/log.hpp"

namespace game::data
{
	/*#####
	# Spell
	#####*/
	Spell::Effects& Spell::_validateEffects(Effects& _effects) const
	{
		if (_effects.empty())
			LOG_WARN("Spell id: " << *id << " has no effects.");
		return _effects;
	}

	Spell::Target& Spell::_validateTarget(Target& _target) const
	{
		if (!m_Flags.contains(Flags::selfCast) && *_target.friendlyMask == 0 && *_target.hostileMask == 0)
			LOG_WARN("Spell id: " << *id << " empty target mask.");
		return _target;
	}

	void Spell::modify(const SpellModifier& _mod)
	{
		assert(_mod.spellId == id);
		if (*_mod.chainEffectivity)
			effectivity = **_mod.chainEffectivity;
		if (*_mod.delay)
			delay = **_mod.delay;
		if (*_mod.projectileId)
			projectileId = **_mod.projectileId;
		if (*_mod.visualId)
			visualId = **_mod.visualId;
		if (_mod.flags)
			setFlags(*_mod.flags);
		if (_mod.target)
			setTarget(*_mod.target);
		if (_mod.area)
			setShape(getShape().modify(*_mod.area));
		if (_mod.effect)
		{
			if (_mod.effect->first < m_Effects.size())
				m_Effects[_mod.effect->first] = _mod.effect->second;
			else
				LOG_ERR("SpellData ID: " << *id << " modify: effect index out of range.");
		}
	}

	void Spell::setShape(Shape _area)
	{
		m_Shape = std::move(_area);
	}

	const Shape& Spell::getShape() const
	{
		return m_Shape;
	}

	void Spell::setEffects(Effects _effects)
	{
		m_Effects = std::move(_validateEffects(_effects));
	}

	const Spell::Effects& Spell::getEffects() const
	{
		return m_Effects;
	}

	void Spell::setFlags(sl2::Bitmask<Flags> _val)
	{
		m_Flags = _val;
	}

	sl2::Bitmask<Spell::Flags> Spell::getFlags() const
	{
		return m_Flags;
	}

	void Spell::setTarget(Target _val)
	{
		m_Target = _validateTarget(_val);
	}

	const Spell::Target& Spell::getTarget() const
	{
		return m_Target;
	}

	void Spell::Effect::setData(Data _data)
	{
		m_Data = std::move(_data);
	}
} // namespace game::data
