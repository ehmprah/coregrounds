#pragma once

namespace game::unit
{
	struct CoreState
	{
		float minHpPercent;
	};

	struct WallState
	{
		int chainBonus;
	};

	struct MinionState
	{
		float attackProgressPercent;
		DirectionVector directionVector;
	};

	struct TowerState
	{
		float attackProgressPercent;
		DirectionVector directionVector;
	};

	struct TriggerState
	{
	};

	struct ProjectileState
	{
		DirectionVector directionVector;
	};

	struct UnitState
	{
		IDs visualIds;
		StatContainer<StatType> stats;
		ConditionHolder conditions;
		AbsPosition pos;
		int level;
		Stat health;
		bool isInWorld;
		unit::State state;

		using DerivedState = std::variant<CoreState, WallState, MinionState, TowerState, TriggerState, ProjectileState>;
		DerivedState derivedState;
	};
} // namespace game::unit