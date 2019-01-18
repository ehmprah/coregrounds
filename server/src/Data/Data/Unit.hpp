#pragma once

#include "Shape.hpp"
#include "Others.hpp"
#include "Core/StatContainer.hpp"

namespace game::data
{
	class UnitModifiers
	{
	public:
		using Modifier = unit::Modifier;

		void setModifier(Modifier _mod, Stat _value);
		Stat getModifier(Modifier _mod) const;
		Stat modifyValue(Modifier _mod, Stat _value) const;

	private:
		using Values = std::array<Stat, static_cast<std::size_t>(unit::Modifier::max) - 1>;
		Values m_Values{};
	};

	struct Unit
	{
	public:
		using StatType = unit::StatType;
		using XpBoostModifier = XpBoostModifier<StatType>;
		using Stats = StatContainer<StatType>;
		using AuraIds = std::vector<ID>;

	private:
		Stats m_Stats;
		XpBoostModifier m_XpBoostModifier;
		AuraIds m_Auras;
		UnitModifiers m_Modifiers;
		Shape m_Body;

	protected:
		Unit() = default;

		Stats& _validateStats(Stats& _stats) const;
		AuraIds& _validateAuraIds(AuraIds& _auras) const;

	public:
		prop::Property<ID> id { prop::Id };
		prop::Property<std::string> name { prop::Name };
		prop::Property<Stat> killingXp { "killing_xp", 0, prop::GreaterEqualZero };
		prop::Property<int> layer{ "layer", 0, prop::GreaterEqualZero };

		void setStats(Stats _stats);
		const Stats& getStats() const;

		void setXpBoostModifier(XpBoostModifier _mods);
		const XpBoostModifier& getXpBoostModifier() const;

		void setAuraIds(AuraIds _auraIds);
		const AuraIds& getAuraIds() const;

		void setModifiers(UnitModifiers _mods);
		const UnitModifiers& getModifiers() const;

		void setBody(Shape _body);
		const Shape& getBody() const;

		virtual std::string_view getTypeName() const = 0;
	};

	struct Core : public Unit
	{
	private:
		using super = Unit;

	public:
		std::string_view getTypeName() const override;
	};

	struct Wall : public Unit
	{
	private:
		using super = Unit;

	public:
		std::string_view getTypeName() const override;
	};

	struct Projectile : public Unit
	{
	public:
		using Flags = sl2::Bitmask<unit::ProjectileFlag>;

	private:
		using super = Unit;

		Flags m_Flags;

	public:
		prop::Property<int> bounces{ "bounces", 0, prop::GreaterEqualZero };
		prop::Property<AbsCoordType> bounceRange{ "bounce_range", 0, prop::GreaterEqualZero };

		void setFlags(Flags _flags);
		Flags getFlags() const;

		std::string_view getTypeName() const override;
	};

	struct Trigger : public Unit
	{
	public:
		using Flags = sl2::Bitmask<unit::TriggerFlag>;

	private:
		using super = Unit;

		Flags m_Flags;

	public:
		prop::Property<Time> lifeTime { "life_time", Time::zero(), prop::DurationGreaterZero };

		void setFlags(Flags _flags);
		Flags getFlags() const;

		std::string_view getTypeName() const override;
	};

	struct ActiveUnit : public Unit
	{
	private:
		using super = Unit;

	protected:
		ActiveUnit() = default;

	public:
		prop::Property<ID> spellId { prop::SpellId };
	};

	struct Tower : public ActiveUnit
	{
	private:
		using super = ActiveUnit;

	public:
		prop::Property<Stat> movementRange { "movement_range", 0, prop::GreaterZero };
		prop::Property<Time> movementWindup { "movement_windup", Time::zero(), prop::DurationGreaterZero };
		prop::Property<Time> movementCooldown { "movement_cooldown", Time::zero(), prop::DurationGreaterZero };

		std::string_view getTypeName() const override;
	};

	struct Minion : public ActiveUnit
	{
	public:
		using Flags = sl2::Bitmask<unit::MinionFlag>;

	private:
		using super = ActiveUnit;

		Flags m_Flags;

	public:
		void setFlags(Flags _flags);
		const Flags& getFlags() const;

		std::string_view getTypeName() const override;
	};
} // namespace game::data
