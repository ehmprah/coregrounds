#include "stdafx.h"
#include "Carrier.hpp"
#include "Game/Entity/Participant.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Entity/MovementGenerator.hpp"

namespace game::spell::carrier
{
	CarrierPtr createCarrier(SpellPtr _spell)
	{
		if (_spell->isProjectile())
			return std::make_unique<Projectile>(std::move(_spell));
		if (_spell->isInstant())
			return std::make_unique<Instant>(std::move(_spell));
		if (_spell->isDelayed())
			return std::make_unique<Delayed>(std::move(_spell));
		return nullptr;
	}

	Interface::Interface(SpellPtr _spell) :
		m_Spell(std::move(_spell))
	{
		assert(m_Spell);
	}

	Spell& Interface::getSpell()
	{
		assert(m_Spell);
		return *m_Spell;
	}

	Instant::Instant(SpellPtr _spell) :
		super(std::move(_spell))
	{
	}

	void Instant::update(Time _diff)
	{
		if (!std::exchange(m_Finished, true))
			getSpell().exec();
	}

	bool Instant::hasFinished() const
	{
		return m_Finished;
	}

	Delayed::Delayed(SpellPtr _spell) :
		super(std::move(_spell))
	{
		assert(getSpell().isDelayed());
		m_Delay.start(*getSpell().getSpellInfo().delay);
	}

	void Delayed::update(Time _diff)
	{
		m_Delay.update(_diff);
		if (m_Delay.getState() == Timer::State::justFinished)
			getSpell().exec();
	}

	bool Delayed::hasFinished() const
	{
		return m_Delay.getState() == Timer::State::finished;
	}

	Projectile::Projectile(SpellPtr _spell) :
		super(std::move(_spell))
	{
		assert(getSpell().isProjectile() && getSpell().getCaster().isUnit());
		auto& unitCaster = getSpell().getCaster().toUnit();
		auto& participant = unitCaster.getOwner();
		auto projectileId = *getSpell().getSpellInfo().projectileId;
		m_Projectile = participant.createProjectile(projectileId, unitCaster.getPosition(), getSpell());
	}

	void Projectile::update(Time _diff)
	{
		if (m_Projectile && m_Projectile->hasFinished())
		{
			m_Projectile->kill();
			m_Projectile = nullptr;
		}
	}

	bool Projectile::hasFinished() const
	{
		return !m_Projectile;
	}
} // namespace game::spell::carrier
