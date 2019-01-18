#include "Aura.hpp"
#include "DataStringHelper.hpp"
#include "Core/HelperFunctions.hpp"
#include "Others.hpp"
#include "Core/log.hpp"

namespace game::data
{
	bool _checkEffect(ID _id, const Aura::Effect& _effect)
	{
		using aura::EffectType;
		switch (_effect.type)
		{
		case EffectType::modStat:
		case EffectType::modStatPercent:
		{
			auto& modStat = _effect.getData<Aura::Effect::ModStat>();
			if (modStat.target == Aura::Effect::Target::none)
			{
				LOG_ERR("Aura id: " << _id << " AuraEffect ModStat: unable to determine target type.");
				return false;
			}

			if (modStat.statType == 0)
			{
				LOG_ERR("Aura id: " << _id << " AuraEffect ModStat: Invalid stat");
				return false;
			}

			if (modStat.group == StatModifierGroup::none)
			{
				LOG_ERR("Aura id: " << _id << " AuraEffect ModStat: Invalid group");
				return false;
			}
			break;
		}
		case EffectType::triggerSpell:
		{
			auto& triggerSpell = _effect.getData<Aura::Effect::TriggerSpell>();
			if (triggerSpell.triggerType == aura::TriggerType::none)
			{
				LOG_ERR("Aura id : " << _id << " AuraEffect TriggerSpell: Invalid trigger_type");
				return false;
			}

			if (triggerSpell.id <= 0)
			{
				LOG_ERR("Aura id : " << _id << " AuraEffect TriggerSpell: Invalid spellId");
				return false;
			}
			break;
		}
		case EffectType::offender:
		{
			auto& offender = _effect.getData<Aura::Effect::Offender>();

			if (offender.percent < 0)
			{
				LOG_ERR("Aura id : " << _id << " AuraEffect Offender: Invalid percent: " << *offender.percent);
				return false;
			}
			break;
		}
		case EffectType::periodicDamage:
		case EffectType::periodicDamagePercent:
		case EffectType::periodicHealing:
		case EffectType::periodicHealingPercent:
		{
			auto& periodicDamage = _effect.getData<Aura::Effect::PeriodicDealing>();

			if (periodicDamage.amount < 0)
			{
				LOG_ERR("Aura id : " << _id << " AuraEffect periodic_dealing: Invalid amount: " << *periodicDamage.amount);
				return false;
			}

			if (periodicDamage.interval->count() < 0)
			{
				LOG_ERR("Aura id : " << _id << " AuraEffect periodic_dealing: Invalid interval: " << *periodicDamage.interval);
				return false;
			}
			break;
		}
		case EffectType::none:
		case EffectType::max:
			LOG_ERR("Aura id: " << _id << " Invalid AuraEffect");
			return false;
			
		}
		return true;
	}

	void Aura::setEffects(const Effects& _effects)
	{
		m_Effects.clear();
		auto id = *this->id;
		std::copy_if(std::begin(_effects), std::end(_effects), std::back_inserter(m_Effects), [id](auto& _effect) {
			return _checkEffect(id, _effect);
		});
	}

	const Aura::Effects& Aura::getEffects() const
	{
		return m_Effects;
	}
} // namespace game::data
