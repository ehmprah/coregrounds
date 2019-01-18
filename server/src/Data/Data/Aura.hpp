#pragma once

#include "Core/SharedDefines.hpp"
#include "Spell.hpp"

namespace game::data
{
	struct Aura
	{
	public:
		struct Effect
		{
			using Type = aura::EffectType;
			Type type = Type::none;

			enum class Target
			{
				none,
				unit,
				device
			};

			struct ModIncSpellEffectivity
			{
				using Target = Effect::Target;
				prop::Property<int> bonus{ "bonus", 0 };
				prop::Property<ID> spellId{ "spellId", 0 };
			};

			struct ModStat
			{
				using Target = Effect::Target;
				prop::Property<Stat> value{ "value", 0 };
				StatModifierGroup group = StatModifierGroup::none;
				prop::Property<uint32_t> statType{ "stat", 0, prop::GreaterEqualZero };
				Target target = Target::none;
			};

			struct TriggerSpell
			{
				prop::Property<ID> id{ "spellId", 0, prop::IdValidator };
				aura::TriggerType triggerType = aura::TriggerType::none;
			};

			struct Offender
			{
				prop::Property<Stat> percent{ "amount", 1, prop::GreaterEqualZero };
			};

			struct PeriodicDealing
			{
				prop::Property<Time> interval{ "interval", Time::zero(), prop::DurationGreaterZero };
				prop::Property<Stat> amount{ "amount", 0, prop::GreaterEqualZero };
			};

			struct PeriodicTrigger
			{
				prop::Property<Time> interval{ "interval", Time::zero(), prop::DurationGreaterZero };
				prop::Property<ID> id{ "spellId", 0, prop::IdValidator };
				prop::Property<bool> perma{ "perma", true };
			};

			struct ApplyCondition
			{
				using Target = Effect::Target;
				Target target = Target::none;
				prop::Property<uint32_t> condition{ "condition", 0, prop::GreaterZero };
			};

			using SpellModifier = data::SpellModifier;

		private:
			using Data = std::variant<ModIncSpellEffectivity, ModStat, TriggerSpell, PeriodicTrigger, SpellModifier, Offender, PeriodicDealing, ApplyCondition>;
			Data m_Data;

		public:
			template <class TData, typename = std::enable_if_t<std::is_convertible_v<TData, Data>>>
			void setData(TData&& _data)
			{
				m_Data = std::forward<TData>(_data);
			}

			template <class TData, typename = std::enable_if_t<std::is_convertible_v<TData, Data>>>
			const TData& getData() const
			{
				return std::get<TData>(m_Data);
			}

			template <class TData, typename = std::enable_if_t<std::is_convertible_v<TData, Data>>>
			void hasData() const
			{
				return std::holds_alternative<TData>(m_Data);
			}
		};
		using Effects = std::vector<Effect>;

	private:
		Effects m_Effects;

	public:
		prop::Property<ID> id { prop::Id };
		prop::Property<std::string> name { prop::Name };
		prop::Property<ID> visualId{ "visualId", 0, prop::GreaterEqualZero };
		prop::Property<Time> duration{ "duration", Time::zero(), prop::DurationGreaterEqualZero };
		prop::Property<int> family{ "family", 0, prop::GreaterEqualZero };
		prop::Property<int> maxStacks{ "max_stacks", 0, prop::GreaterEqualZero };
		sl2::Bitmask<aura::AuraFlags> flags;

		void setEffects(const Effects& _effects);
		const Effects& getEffects() const;
	};
} // namespace game::data
