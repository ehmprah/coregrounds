#pragma once

#include "AuraEffects.hpp"

namespace game::aura
{
	class AuraManager;
	class Aura :
		sl2::NonCopyable
	{
	private:
		effect::Effects m_Effects;
		std::vector<std::pair<effect::Interface*, Timer>> m_PeriodicEffects;
		const data::Aura& m_AuraInfo;
		Castable& m_Target;
		CastableSnapshot m_CasterInfo;
		Timer m_Duration;
		int m_Stacks = 0;
		uint32_t m_EventTypes = 0;

		bool m_IsErasable = false;

		void _calculateEventTypes();
		void _doEffects(bool _apply);
		void _startDurationIfNecessary();
		void _findPeriodicEffects();

	public:
		Aura(Castable& _target, const CastableSnapshot& _casterInfo, const data::Aura& _auraInfo, effect::Effects _effects);

		void notify(Event& _event);
		std::uint32_t getEventTypes() const;
		bool isNotifiable() const;

		int getFamily() const;
		bool isStackable() const;
		int getMaxStacks() const;
		int getStacks() const;
		void setStacks(int _stacks);
		Time getDuration() const;
		void setDuration(Time _time);

		const data::Aura& getAuraInfo() const;
		Castable& getTarget() const;
		CastableHandle getCaster() const;
		const CastableSnapshot& getCasterInfo() const;

		void update(Time _diff);

		bool isTimed() const;
		bool isPersistent() const;
		bool isExpired() const;
		bool isErasable() const;
		void setErasable();

		void applyEffects();
		void removeEffects();

		const effect::Effects& getEffects() const;
	};
	using AuraPtr = std::unique_ptr<Aura>;

	AuraPtr createAura(const data::Aura& _auraInfo, Castable& _target, const CastableSnapshot& _casterInfo);
	AuraPtr createAura(const data::Aura& _auraInfo, effect::Effects _effects, Castable& _target, const CastableSnapshot& _casterInfo);
} // namespace game::aura
