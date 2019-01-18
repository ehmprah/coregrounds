#pragma once

#include "Core/SharedDefines.hpp"
#include "Shape.hpp"
#include "PropertyDefines.hpp"

namespace game::data
{
	struct SpellModifier;
	struct Spell
	{
	public:
		struct Effect
		{
		public:
			using Type = spell::EffectType;
			Type type = Type::none;

			struct Value
			{
				prop::Property<int> value{ "value", 0 };
			};

			struct Id
			{
				prop::Property<ID> value{ prop::Id };
			};

			struct Percentage
			{
				prop::Property<Stat> value{ "percent", 0 };
			};

			struct TriggerSpell
			{
				prop::Property<ID> id{ prop::Id };
				prop::Property<bool> forward{ "forward", false };
			};
			using Data = std::variant<Value, Id, Percentage, TriggerSpell>;

		private:
			Data m_Data;

			template <class T>
			using _IsDataType = std::disjunction<std::is_same<T, Value>, std::is_same<T, Id>,
				std::is_same<T, Percentage>, std::is_same<T, TriggerSpell>>;
			template <class T>
			using IsDataType = _IsDataType<std::remove_cv_t<T>>;

		public:
			void setData(Data _data);
			template <class TData, typename = std::enable_if_t<IsDataType<TData>::value>>
			const TData& getData() const
			{
				return std::get<TData>(m_Data);
			}

			template <class TData, typename = std::enable_if_t<IsDataType<TData>::value>>
			bool hasData() const
			{
				return std::holds_alternative<TData>(m_Data);
			}
		};
		using Effects = std::vector<Effect>;

		struct Target
		{
			using Flags = game::spell::TargetFlags;
			sl2::Bitmask<Flags> friendlyMask{ Flags::all };
			sl2::Bitmask<Flags> hostileMask{ Flags::all };
			prop::Property<int> count{ "count", 0 };
		};

		using Flags = game::spell::SpellFlags;

	private:
		Shape m_Shape;
		Effects m_Effects;
		sl2::Bitmask<Flags> m_Flags;
		Target m_Target;

		Effects& _validateEffects(Effects& _effects) const;
		Target& _validateTarget(Target& _target) const;

	public:
		prop::Property<ID> id { prop::Id };
		prop::Property<std::string> name { prop::Name };
		prop::Property<Stat> effectivity { "effectivity", 0 };
		prop::Property<Time> delay { "delay", Time::zero(), prop::DurationGreaterEqualZero };
		prop::Property<ID> projectileId { "projectileId", 0, prop::GreaterEqualZero };
		prop::Property<ID> visualId { "visualId", 0, prop::GreaterEqualZero };

		void modify(const SpellModifier& _mod);

		void setShape(Shape _area);
		const Shape& getShape() const;

		void setEffects(Effects _effects);
		const Effects& getEffects() const;

		void setFlags(sl2::Bitmask<Flags> val);
		sl2::Bitmask<Flags> getFlags() const;

		void setTarget(Target _val);
		const Target& getTarget() const;
	};

	struct SpellModifier
	{
		prop::Property<ID> spellId{ prop::SpellId };
		prop::Property<std::optional<Stat>> chainEffectivity{ "chain_effectivity" };
		prop::Property<std::optional<Time>> delay{ "delay" };
		prop::Property<std::optional<ID>> projectileId{ "projectileId" };
		prop::Property<std::optional<ID>> visualId{ "visualId" };

		using Flags = game::spell::SpellFlags;
		std::optional<sl2::Bitmask<Flags>> flags;

		std::optional<std::pair<std::size_t, Spell::Effect>> effect;
		std::optional<Spell::Target> target;
		std::optional<AreaModifier> area;
	};
} // namespace game::data
