#pragma once

#include "Data/Spell.hpp"

namespace game
{
	class Castable;
namespace spell
{
	class Spell;

	using Targets = std::vector<Castable*>;

namespace effect
{
	class Interface : sl2::NonCopyable
	{
	private:
		const data::Spell::Effect& m_EffectInfo;

		virtual void _execOnTarget(const Spell& _spell, Castable& _target) = 0;

	public:
		Interface(const data::Spell::Effect& _effectInfo);
		virtual ~Interface() = default;

		const data::Spell::Effect& getInfo() const;

		virtual void exec(const Spell& _spell, Targets& _targets);
	};
	using EffectPtr = std::unique_ptr<Interface>;
	using Effects = std::vector<EffectPtr>;

	class InstantKill : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override;

	public:
		InstantKill(const data::Spell::Effect& _effect);
	};

	class DealDamage : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override;

	public:
		DealDamage(const data::Spell::Effect& _effect);
	};

	class DealHealing : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override;

	public:
		DealHealing(const data::Spell::Effect& _effect);
	};

	class DealHealthAsDamage : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override;

	public:
		DealHealthAsDamage(const data::Spell::Effect& _effect);
	};

	class ApplyAura : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override {};

	public:
		ApplyAura(const data::Spell::Effect& _effect);

		void exec(const Spell& _spell, Targets& _targets) override;
	};

	class TriggerSpell : public Interface
	{
	private:
		using super = Interface;

		void _trigger(const Spell& _spell, Castable& _target, data::Spell _spellData);
		void _execOnTarget(const Spell& _spell, Castable& _target) override {}	// not used here

	public:
		TriggerSpell(const data::Spell::Effect& _effect);

		void exec(const Spell& _spell, Targets& _targets) override;
	};

	class SpawnMinion : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override {} // not used here

	public:
		SpawnMinion(const data::Spell::Effect& _effect);

		void exec(const Spell& _spell, Targets& _targets) override;
	};

	class SpawnTrigger : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override {} // not used here

	public:
		SpawnTrigger(const data::Spell::Effect& _effect);

		void exec(const Spell& _spell, Targets& _targets) override;
	};

	class ChangeOwner : public Interface
	{
	private:
		using super = Interface;

		void _execOnTarget(const Spell& _spell, Castable& _target) override;

	public:
		ChangeOwner(const data::Spell::Effect& _effect);
	};

	EffectPtr createEffect(const data::Spell::Effect& _effect);
} // namespace effect
} // namespace spell
} // namespace game
