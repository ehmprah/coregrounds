#pragma once

#include "Core/SharedDefines.hpp"
#include "Core/SLInclude.hpp"
#include "DataFwd.hpp"
#include "Others.hpp"
#include "Property.hpp"
#include "Unit.hpp"
#include "Spell.hpp"
#include "Aura.hpp"
#include "Factory.hpp"
#include "Ability.hpp"
#include "AiStrategy.hpp"

namespace game::data
{
	struct GameProperties
	{
	private:
		void _checkAndAdjustUpgradeDurations();
		void _checkAndAdjustXPCurve();

	public:
		struct Game
		{
			prop::Property<int> minionLimit{ "minion_limit", 30, prop::GreaterZero };
			prop::Property<ID> minionLimitExplodeSpellID{ "minion_limit_explode_spellId", 0, prop::GreaterZero };
			prop::Property<ID> minionCoreSuicideSpellID{ "minion_core_suicide_spellId", 0, prop::GreaterZero };
			prop::Property<int> wallChainArmorBonus{ "wall_chain_armor_bonus", 20, prop::GreaterEqualZero };
			prop::Property<int> maxWallChain{ "max_wall_chain", 10, prop::GreaterEqualZero };
			prop::Property<int> initialWalls{ "initial_walls", 7, prop::GreaterEqualZero };
			prop::Property<Time> unitRegenerationInterval{ "unit_regeneration_interval", Time(1000), prop::DurationGreaterZero };
			prop::Property<Time> warumUpTime{ "warm_up_time", Time(1000), prop::DurationGreaterEqualZero };

			// xp stuff
			prop::Property<Stat> xpPerSecond{ "xp_per_second", 0, prop::GreaterEqualZero };
			prop::Property<int> xpPerLevel{ "xp_per_level", 150, prop::GreaterZero };
			prop::Property<int> maxLevel{ "max_level", 20, prop::GreaterZero };
			prop::Property<Stat> xpHandicapPerUpgrade{ "xp_handicap_per_upgrade", 0.1, prop::GreaterEqualZero };
			XPCurve xpCurve;
			XpBoostModifier<unit::StatType> unitXpBoostModifier;
			XpBoostModifier<device::StatType> deviceXpBoostModifier;

			UnitModifiers unitModifiers;
			Unit::Stats unitStats;
			Device::Stats deviceStats;
			//UpgradeDurations upgradeDurations;
			prop::Property<Time> towerDeathCooldown{ "tower_death_cooldown", Time(7000), prop::DurationGreaterZero };

			// fixed
			static const Size<TileCoordinateType> fieldSize;

#ifndef NDEBUG
			struct Debug
			{
				struct Player
				{
					std::vector<TilePosition> wallLocations;
				};
				std::array<Player, 2> players;

				prop::Property<bool> emptyAI{ "empty_ai", false };

			};

			std::optional<Debug> debug;
#endif
		} game;

		struct Pick
		{
			prop::Property<Time> maxTime{ "max_time", std::chrono::duration_cast<Time>(std::chrono::seconds(60)), prop::DurationGreaterZero };
			prop::Property<Time> modPageSelectionTime{ "mod_page_selection_time", std::chrono::duration_cast<Time>(std::chrono::seconds(30)), prop::DurationGreaterZero };
			prop::Property<int> maxFactoryBans{ "max_factory_bans", 1, prop::GreaterEqualZero };
			prop::Property<int> maxAbilityBans{ "max_ability_bans", 1, prop::GreaterEqualZero };

#ifndef NDEBUG
			struct Debug
			{
				struct Player
				{
					// we ignore wall factory here
					std::array<prop::Property<ID>, MAX_FACTORIES - 1> factories{ {
						{ "factory0", 0, prop::GreaterEqualZero },
						{ "factory1", 0, prop::GreaterEqualZero },
						{ "factory2", 0, prop::GreaterEqualZero },
						{ "factory3", 0, prop::GreaterEqualZero },
						{ "factory4", 0, prop::GreaterEqualZero }
					} };

					std::array<prop::Property<ID>, MAX_ABILITIES> abilities{ {
						{ "ability0", 0, prop::GreaterEqualZero },
						{ "ability1", 0, prop::GreaterEqualZero },
						{ "ability2", 0, prop::GreaterEqualZero }
						} };
				};
				std::array<Player, 2> players;
			};

			std::optional<Debug> debug;
#endif
		} pick;

		struct Finalize
		{
			prop::Property<Time> phaseTime{ "phase_time", std::chrono::duration_cast<Time>(std::chrono::seconds(10)), prop::DurationGreaterZero };
		} finalize;

		void checkAndAdjust();
	};

	class Game :
		sl2::NonCopyable
	{
	public:
		GameProperties gameProperties;
		
		Table<Color> colors;
		Table<PlayerIcon> playerIcons;
		Table<Skin> skins;
		Table<Background> backgrounds;
		Table<Emote> emotes;
		Table<Modification> modifications;

		Table<Wall> walls;
		Table<Core> cores;
		Table<Tower> towers;
		Table<Minion> minions;
		Table<Projectile> projectiles;
		Table<Trigger> triggers;

		Table<Ability> abilities;
		Table<Factory> factories;

		Table<Aura> auras;
		Table<Spell> spells;

		Table<Unlock> unlocks;

		// ai
		Table<AiMazeLayout> aiMazeLayouts;
		Table<AiStrategy> aiStrategies;

		void checkAndAdjust();
	};

namespace
{
	const auto idLessCompare = [](const auto& _lhs, ID _rhs)
	{
		return *_lhs.id < _rhs;
	};
} // namespace anonymous

	template <class T>
	const T* findData(const Table<T>& _container, ID _id)
	{
		auto itr = std::lower_bound(std::begin(_container), std::end(_container), _id, idLessCompare);
		return (itr != std::end(_container) && *itr->id == _id) ? &*itr : nullptr;
	}

	template <class T, class Obj>
	bool insertData(Table<T>& _container, Obj _obj)
	{
		if (*_obj.id == 0)
			return false;
		auto itr = std::lower_bound(std::begin(_container), std::end(_container), *_obj.id, idLessCompare);
		if (itr == std::end(_container) || *itr->id != *_obj.id)
		{
			_container.emplace(itr, std::move(_obj));
			return true;
		}
		return false;
	}
} // namespace game::data
