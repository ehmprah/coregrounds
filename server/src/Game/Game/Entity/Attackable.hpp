#pragma once

#include "Locatable.hpp"
#include "GameDependencies.hpp"

namespace game::unit
{
	class Attackable
	{
	public:
		Attackable(const data::ActiveUnit& _info, const TargetPreference* _preference, Unit& _owner, GameDependencies& _gameDependencies);
		virtual ~Attackable() = default;

		void restartAttack();
		const TargetPreference* getTargetPreference() const;
		bool hasAttackSpell() const;
		float getAttackProgressPercent() const;
		virtual AbsCircle getAttackArea() const = 0;

		CastableHandle getTarget() const;

	protected:
		void updateAttackable(Time _diff);

	private:
		GameDependencies& m_GameDependencies;
		const data::ActiveUnit& m_Info;
		Unit& m_Owner;
		const data::Spell* m_AttackSpell = nullptr;

		const TargetPreference* m_TargetPreference = nullptr;
		CastableHandle m_Target;

		Timer m_AttackTimer;

		void _startAttackTimer();
		CastableHandle _findTarget() const;
	};
} // namespace game::unit
