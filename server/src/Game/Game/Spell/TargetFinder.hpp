#pragma once

#include "Data/Spell.hpp"

namespace game::spell
{
	class TargetFinder :
		sl2::NonCopyable
	{
	private:
		using Unit = unit::Unit;
		using Castable = Castable;

		const Participant& m_Participant;
		using TargetDefinition = ::game::data::Spell::Target;

		std::vector<Unit*> _getUniqueTargets(const TargetDefinition& _targetInfo, const AbsPosition& _pos, const AbsShape* _shape) const;
		void _findUnits(const TargetDefinition& _targetInfo, const AbsPosition& _pos, std::vector<Unit*>& _targets, const AbsShape* _shape) const;
		void _findFactories(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const;
		void _findAbilities(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const;

	public:
		TargetFinder(const Participant& _participant);

		std::vector<Unit*> findUnits(const TargetDefinition& _targetInfo, const AbsShape& _shape) const;
		std::vector<Unit*> findUnits(const TargetDefinition& _targetInfo, const AbsPosition& _pos, const ::game::data::Shape& _area) const;
		std::vector<Castable*> findDevices(const TargetDefinition& _targetInfo) const;

		bool hasCorrectTargetFlags(const ::game::data::Spell::Target& _targetInfo, const Castable& _target) const;
	};
} // namespace game::spell
