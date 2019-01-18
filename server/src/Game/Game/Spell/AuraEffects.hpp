#pragma once

#include "Data/Aura.hpp"
#include "Game/Entity/LifeLink.hpp"
#include "AuraFwd.hpp"

namespace game::aura
{
	using Targets = std::vector<Castable*>;

namespace effect
{
	class Interface :
		sl2::NonCopyable
	{
	private:
		virtual void _notify(const Aura& _aura, Event& _event) {};

	public:
		virtual ~Interface() = default;

		void notify(const Aura& _aura, Event& _event);
		virtual EventType getEventType() const { return EventType::none; }

		// this value must be constant for each derived class. Different values will lead to undefined behavior
		virtual bool isPeriodic() const { return false; }
		virtual Time getIntervalTime() const { return Time(0); }

		virtual void apply(const Aura& _aura);
		virtual void remove(const Aura& _aura);
		virtual void tick(const Aura& _aura) {}		// this gets just called, if isPeriodic() returns true
	};
	using EffectPtr = std::shared_ptr<Interface>;
	using Effects = std::vector<EffectPtr>;

	class ApplyCondition :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::ApplyCondition& m_Data;

		void _do(const Aura& _aura, bool _apply);

	public:
		ApplyCondition(const data::Aura::Effect::ApplyCondition& _data);

		void apply(const Aura& _aura) override;
		void remove(const Aura& _aura) override;
	};

	class Dummy :
		public Interface
	{
	private:
		using super = Interface;

	public:
	};

	class LifeLink :
		public Interface
	{
	private:
		using super = Interface;

		unit::LifeLinkContainer m_LifeLinkContainer;

		void _notify(const Aura& _aura, Event& _event) override;

	public:
		void apply(const Aura& _aura) override;
		void remove(const Aura& _aura) override;

		EventType getEventType() const override;
	};

	class ModIncSpellEffectivity :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::ModIncSpellEffectivity& m_Data;

		void _notify(const Aura& _aura, Event& _event) override;

	public:
		ModIncSpellEffectivity(const data::Aura::Effect::ModIncSpellEffectivity& _data);

		EventType getEventType() const override;
	};

	class ModifySpell :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::SpellModifier& m_Data;

		void _notify(const Aura& _aura, Event& _event) override;

	public:
		ModifySpell(const data::Aura::Effect::SpellModifier& _data);

		EventType getEventType() const override;
	};

	class ModStat :
		public Interface
	{
	private:
		using super = Interface;

		bool m_Percent = false;
		const data::Aura::Effect::ModStat& m_Data;

		void _do(const Aura& _aura, bool _apply);

	public:
		ModStat(const data::Aura::Effect::ModStat& _data, bool _percent);

		StatModifierGroup getModifierGroup() const;

		void apply(const Aura& _aura) override;
		void remove(const Aura& _aura) override;
	};

	class Offender :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::Offender& m_Data;

		void _notify(const Aura& _aura, Event& _event) override;

	public:
		Offender(const data::Aura::Effect::Offender& _data);

		EventType getEventType() const override;
	};

	class PeriodicDealing :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::PeriodicDealing& m_Data;
		bool m_Percent;
		bool m_Damage;

	public:
		PeriodicDealing(const data::Aura::Effect::PeriodicDealing& _data, bool _damage, bool _percent);

		void tick(const Aura& _aura) override;
		bool isPeriodic() const override;
		Time getIntervalTime() const override;
	};

	class PeriodicTrigger :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::PeriodicTrigger& m_Data;

	public:
		PeriodicTrigger(const data::Aura::Effect::PeriodicTrigger& _data);

		void tick(const Aura& _aura) override;
		bool isPeriodic() const override;
		Time getIntervalTime() const override;
	};

	class TriggerSpell :
		public Interface
	{
	private:
		using super = Interface;

		const data::Aura::Effect::TriggerSpell& m_Data;

		void _notify(const Aura& _aura, Event& _event) override;

	public:
		TriggerSpell(const data::Aura::Effect::TriggerSpell& _data);

		EventType getEventType() const override;
	};

	EffectPtr createEffect(const data::Aura::Effect& _effect);
	Effects createEffects(const data::Aura::Effects& _effectsDef);
} // namespace effect
} // namespace game::aura
