#pragma once

#include "Game/Spell/Spell.hpp"
#include "Game/Spell/Aura.hpp"

namespace game
{
	class AuraHolder :
		sl2::NonCopyable
	{
	private:
		std::vector<aura::AuraPtr> m_Auras;
		std::vector<aura::Aura*> m_NotifiableAuras;

		void _remove(aura::Aura& _aura);
		std::vector<aura::Aura*> _findIf(std::function<bool(const aura::Aura&)> _predicate) const;

	public:
		void notify(aura::Event& _event);
		void update(Time _diff);

		void add(aura::AuraPtr _aura);
		void clear();

		std::vector<const aura::Aura*> findAuras(std::function<bool(const aura::Aura&)> _predicate) const;
	};

	struct CastableInitializer
	{
		GameDependencies& gameDependencies;
		const spell::TargetFinder& targetFinder;
	};

	class Castable
	{
		friend class spell::CastableDestination;		// we need the friend here for the internal spell counter
		friend class spell::SelfDestination;
		friend class spell::Spell;
	private:
		GameDependencies& m_GameDependencies;
		const spell::TargetFinder* m_TargetFinder;

		AuraHolder m_AuraHolder;
		sl2::MasterHandle<Castable> m_MasterHandle;
		int m_IncomingSpellCounter = 0;
		int m_OutgoingSpellCounter = 0;

		template <class TargetData>
		bool _cast(data::Spell _spellInfo, CastableSnapshot _snapshot, TargetData _data, bool _triggered)
		{
			if (_triggered || canCast())
			{
				return m_GameDependencies.spellManager.addSpell(spell::createSpell(std::move(_spellInfo), m_GameDependencies,
					getTargetFinder(), *this, _snapshot, _data, _triggered));
			}
			return false;
		}
	
	protected:
		void updateCastable(Time _diff);
		virtual void setupCastableSnapshot(CastableSnapshot& _info) const;
		void clearAuras();
		void setTargetFinder(const spell::TargetFinder& _targetFinder);
	
	public:
		Castable(const CastableInitializer& _initializer);

		const spell::TargetFinder& getTargetFinder() const;
		GameDependencies& getGameDependencies() const;

		CastableHandle getCastableHandle() const;
		CastableSnapshot generateCastableInfo() const;
	
		virtual bool canCast() const;
		virtual bool cast(data::Spell _spellInfo, AbsPosition _pos);
		virtual bool cast(data::Spell _spellInfo, CastableHandle _target);
		virtual bool castTriggered(data::Spell _spellInfo, CastableSnapshot _snapshot, AbsPosition _pos);
		virtual bool castTriggered(data::Spell _spellInfo, CastableSnapshot _snapshot, CastableHandle _target);

		bool isAffectedBySpell() const;
	
		void addAura(aura::AuraPtr _aura);
		void notifyAuras(aura::Event& _event);
		std::vector<const aura::Aura*> findAuras(std::function<bool(const aura::Aura&)> _predicate) const;

		IDs gatherAuraVisualIds() const;

		// RTTI
		virtual bool isParticipant() const = 0;

		virtual bool isUnit() const = 0;
		virtual const unit::Unit& toUnit() const = 0;
		virtual unit::Unit& toUnit() = 0;

		virtual bool isDevice() const = 0;
		virtual const device::Device& toDevice() const = 0;
		virtual device::Device& toDevice() = 0;
	};
} // namespace game
