#pragma once

#include "Data/Spell.hpp"
#include "Entity/DamageInfo.hpp"

namespace game::aura::event
{
	class Interface
	{
	public:
		virtual ~Interface() = default;

		virtual EventType getType() const = 0;
	};

	class Trigger : public Interface
	{
	private:
		TriggerType m_TriggerType;
		Castable& m_Owner;
		Castable& m_Trigger;

	public:
		Trigger(Castable& _owner, Castable& _trigger, TriggerType _triggerType);

		Castable& getOwner() const;
		Castable& getTrigger() const;
		TriggerType getTriggerType() const;
		EventType getType() const override;
	};

	class CreateSpell : public Interface
	{
	private:
		std::vector<const data::SpellModifier*> m_SpellMods;
		Castable& m_Owner;

	public:
		CreateSpell(Castable& _owner);

		Castable& getOwner() const;
		const std::vector<const data::SpellModifier*>& getSpellMods() const;
		void addSpellMod(const data::SpellModifier& _mod);
		EventType getType() const override;
	};

	class IncomingSpell : public Interface
	{
	private:
		CastableHandle m_Caster;
		ID m_SpellId;
		const Castable& m_Owner;
		int m_Bonus = 0;

	public:
		IncomingSpell(const Castable& _owner, CastableHandle _caster, ID _spellId);

		void addBonus(int _bonus);
		int getBonus() const;
		const CastableHandle& getCaster() const;
		ID getSpellId() const;
		const Castable& getOwner() const;
		EventType getType() const override;
	};

	class CalculateDamage : public Interface
	{
	private:
		Castable& m_Owner;
		unit::DamageInfo& m_DamageInfo;

	public:
		CalculateDamage(Castable& _owner, unit::DamageInfo& _damageInfo);

		Castable& getOwner() const;
		unit::DamageInfo& getDamageInfo();
		EventType getType() const override;
	};

	class AppliedXp : public Interface
	{
	public:
		enum class Reason
		{
			damage,
			heal,
			kill
		};

	private:
		Castable& m_Owner;		// unit which provides the xp
		Castable* m_Trigger;	// unit which gains the xp; if not set it doesn't exists anymore

		Stat m_XpAmount = 0;
		Reason m_Reason = Reason::damage;

	public:
		AppliedXp(Castable& _owner, Castable* _trigger, Stat _xpAmount, Reason _reason);

		Stat getXpAmount() const;
		Castable& getOwner() const;
		Castable* getTrigger() const;
		Reason getReason() const;
		EventType getType() const override;
	};
} // namespace game::aura::event
