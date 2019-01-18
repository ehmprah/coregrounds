#pragma once

#include "Spell.hpp"

namespace game::spell::carrier
{
	class Interface :
		sl2::NonCopyable
	{
	private:
		SpellPtr m_Spell;

	protected:
		Spell& getSpell();

	public:
		Interface(SpellPtr _spell);
		virtual ~Interface() = default;

		virtual void update(Time _diff) = 0;
		virtual bool hasFinished() const = 0;
	};
	using CarrierPtr = std::unique_ptr<Interface>;

	class Instant :
		public Interface
	{
	private:
		using super = Interface;

		bool m_Finished = false;

	public:
		Instant(SpellPtr _spell);

		void update(Time _diff) override;
		bool hasFinished() const override;
	};

	class Delayed :
		public Interface
	{
	private:
		using super = Interface;

		Timer m_Delay;

	public:
		Delayed(SpellPtr _spell);

		void update(Time _diff) override;
		bool hasFinished() const override;
	};

	class Projectile :
		public Interface
	{
	private:
		using super = Interface;

		unit::Projectile* m_Projectile = nullptr;

	public:
		Projectile(SpellPtr _spell);

		void update(Time _diff) override;
		bool hasFinished() const override;
	};

	CarrierPtr createCarrier(SpellPtr _spell);
} // namespace game::spell::carrier
