#pragma once

#include "SpellEffects.hpp"
#include "Game/Entity/Locatable.hpp"
#include "Destination.hpp"
#include "GameDependencies.hpp"

namespace game::spell
{
	class Spell :
		sl2::NonCopyable
	{
	private:
		data::Spell m_SpellInfo;
		GameDependencies& m_GameDependencies;
		const TargetFinder& m_TargetFinder;
		Castable& m_Caster;
		CastableSnapshot m_CasterInfo;
		DestinationPtr m_Destination;
		effect::Effects m_Effects;
		data::Shape m_Area;
		int m_Executions = 0;
		bool m_Triggered = false;

		void _setupArea();
		void _setupTarget();
		void _doEffects(Targets& _targets) const;

	public:
		Spell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
			CastableSnapshot _casterInfo, effect::Effects _effects, DestinationPtr _target, bool _triggered);

		~Spell();

		GameDependencies& getGameDependencies() const;
		const TargetFinder& getTargetFinder() const;
		const CastableSnapshot& getCasterInfo() const;
		const data::Spell& getSpellInfo() const;
		Castable& getCaster() const;
		std::optional<AbsPosition> getDestinationPosition() const;
		CastableHandle getDestinationEntity() const;
		bool changeDestination(const AbsShape& _area, bool _uniqueCheck);
		DestinationPtr copyDestination() const;
		const data::Shape& getShape() const;

		bool isValid() const;
		void exec();
		int getExecutions() const;

		bool isTriggered() const;
		bool isDelayed() const;
		bool isInstant() const;
		bool isProjectile() const;
	};
	using SpellPtr = std::unique_ptr<Spell>;

	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, AbsPosition _pos, bool _triggered);
	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, CastableHandle _target, bool _triggered);
	SpellPtr createSpell(data::Spell _spellInfo, GameDependencies& _gameDependencies, const TargetFinder& _targetFinder, Castable& _caster,
		CastableSnapshot _casterInfo, DestinationPtr _target, bool _triggered);
} // namespace game::spell
