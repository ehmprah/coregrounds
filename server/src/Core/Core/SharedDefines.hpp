#pragma once

#include "Core/Shape.hpp"
#include "Core/StdInclude.hpp"
#include "Core/SLInclude.hpp"

namespace game
{
	using ID = unsigned int;
	using IDs = std::vector<ID>;
	using Time = std::chrono::milliseconds;

	inline constexpr double Epsilon = std::numeric_limits<double>::epsilon();

	template <class T>
	using Vector2D = sl2::geometry2d::Vector<T>;
	template <class T>
	using Size = sl2::geometry2d::Size<T>;
	using AbsCoordType = double;
	using AbsSize = Size<AbsCoordType>;
	using AbsPosition = Vector2D<AbsCoordType>;
	using DirectionVector = Vector2D<AbsCoordType>;
	using TileCoordinateType = int;
	using TilePosition = Vector2D<TileCoordinateType>;
	static const AbsCoordType TileAbsoluteSize = 1;

	using AbsShape = Shape<AbsCoordType>;
	template <class T>
	using Line = sl2::geometry2d::Line<T>;
	using AbsLine = Line<AbsCoordType>;
	template <class T>
	using Rect = sl2::geometry2d::Rect<T>;
	using AbsRect = Rect<AbsCoordType>;
	using TileRect = Rect<TileCoordinateType>;
	template <class T>
	using Circle = sl2::geometry2d::Circle<T>;
	using AbsCircle = Circle<AbsCoordType>;

	constexpr unsigned int MAX_ABILITIES = 3;
	constexpr unsigned int MAX_FACTORIES = 6;

	constexpr const char* MatchTypes[]
	{
		"bot",
		"casual",
		"ranked",
		"random",
		"private"
	};

	using Stat = float;

	enum class StatModifierGroup
	{
		none = 0,
		modification = 1,
		upgrade = 2,
		buff = 3,

		max
	};

	enum class ParticipantProgress
	{
		none = 0x0000,
		settled = 0x0001			// placed the initial walls
	};
	using ParticipantProgressFlags = sl2::Bitmask<ParticipantProgress>;

	enum class UnlockType
	{
		none,
		factory,
		ability,
		modification,
		color,
		skin,
		playerIcon,
		background,
		emote,

		max
	};

	using Unlocks = std::array<IDs, static_cast<std::size_t>(UnlockType::max) - 1>;
	using ModPage = std::tuple<ID/*pid*/, IDs/*modIds*/>;
} // namespace game

namespace game::device
{
	constexpr const int MaxFactoryUpgradeTypes = 3;
	constexpr const int MaxFactoryUpgrades = 4;

	enum class AbilityFlag
	{
		none = 0,
		requireUnitTarget = 0x001,

		max
	};

	enum class StatType
	{
		none,
		spawnSpeed,
		stacks,
		xp,
		cooldownSpeed,

		max
	};

	enum class FactoryType
	{
		none,
		wall,
		tower,
		minion,

		max
	};

	static const Time SpawnSpeedBase{ 1000 };
} // namespace game::device

namespace game::unit
{
	enum class State
	{
		justSpawned = 0,
		alive,
		justDied,
		dead,
	};

	enum class Condition
	{
		none			= 0,
		root,
		daze,
		invulnerable,
		silence,

		max
	};

	using ConditionHolder = std::array<int, static_cast<std::size_t>(Condition::max)>;

	enum class StatType
	{
		none,
		armor,
		armorPenetration,
		attackRange,
		attackSpeed,
		damage,
		damageTaken,
		healing,
		maxHealth,
		moveSpeed,
		regeneration,
		thorns,
		visionRange,
		//lifeSteal,

		max
	};

	enum class Type
	{
		none,
		core,
		wall,
		tower,
		minion,
		projectile,
		trigger,

		max
	};

	enum class Modifier
	{
		none,
		xpOnDamageTaken,
		xpOnDamageDealt,
		xpOnUnitKilled,
		xpOnHealingDealt,

		damageAgainstCores,
		damageAgainstMinions,
		damageAgainstTowers,
		damageAgainstWalls,
		damageAgainstCC,

		healGained,

		max
	};

	enum class Flags
	{
		alwaysVisible				= 0x0001,
		unsolid						= 0x0002,
		noVisibilityModification	= 0x0004
	};

	enum class MinionFlag
	{
		none				= 0x0000,
		unsolid				= 0x0001,
	};

	enum class TriggerFlag
	{
		none						= 0x0000,
		modifyVisibility			= 0x0001
	};

	enum class ProjectileFlag
	{
		none						= 0x0000,
		uniqueHit					= 0x0001
	};

	static const Time AttackSpeedBase{ 1000 };
	static const Time MovementSpeedBase{ 1000 };
} // namespace game::unit

namespace game::unit::targetPreference
{
	enum class Target
	{
		none = 0x000,
		core = 0x001,
		wall = 0x002,
		tower = 0x004,
		minion = 0x008,
		all = core | wall | tower | minion,

		max
	};

	enum class Mode
	{
		none,
		low,
		high,
		first,
		random,

		max
	};

	enum class Flag
	{
		none		= 0x00,
		damaged		= 0x01,


		all			= damaged,

		max
	};
} // namespace game::unit::targetPreference

namespace game::spell
{
	enum class EffectType
	{
		none,
		applyAura,
		damage,
		healing,
		instantKill,
		healthAsDamage,
		triggerSpell,
		spawnMinion,
		spawnTrigger,
		changeOwner,

		max
	};

	enum class SpellFlags
	{
		none					= 0,
		selfCast				= 0x001,
		casterNoTarget			= 0x002,
		tileAligned				= 0x004,
		unitRangeAsAreaSize		= 0x008,

		max
	};

	enum class TargetFlags
	{
		none				= 0,
		core				= 0x001,
		minion				= 0x002,
		tower				= 0x004,
		wall				= 0x008,
		unit				= core | minion | tower | wall,

		minion_factory		= 0x010,
		tower_factory		= 0x020,
		wall_factory		= 0x040,
		factory				= minion_factory | tower_factory | wall_factory,
		ability				= 0x080,
		device				= factory | ability,

		all					= unit | device,

		max
	};
} // namespace game::spell
//
namespace game::aura
{
	enum class AuraFlags
	{
		none					= 0,
		individualCasterStack	= 0x001,
		accumulateDuration		= 0x002,

		max
	};

	enum class EffectType
	{
		none,
		modStat,
		modStatPercent,
		condition,
		triggerSpell,
		lifeLink,
		modIncSpell,
		modSpell,
		offender,
		dummy,
		periodicTrigger,
		periodicDamage,
		periodicDamagePercent,
		periodicHealing,
		periodicHealingPercent,

		max
	};

	enum class TriggerType
	{
		none,
		died,
		summonedUnit,
		
		max
	};

	enum class EventType
	{
		none				= 0x0000,
		trigger				= 0x0001,
		createSpell			= 0x0002,
		calculateDamage		= 0x0004,
		applyDamage			= 0x0008,
		applyXp				= 0x0010,
		incomingSpell		= 0x0020,

		max
	};
	inline constexpr unsigned int EventTypeCount = 4;
} // namespace game::aura
