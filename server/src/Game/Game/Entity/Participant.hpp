#pragma once

#include "Factory.hpp"
#include "Ability.hpp"
#include "Game/Map/VisibilityMap.hpp"
#include "Game/Map/BuildingMap.hpp"
#include "Game/Map/RoutingMap.hpp"
#include "Game/Spell/TargetFinder.hpp"

namespace game
{
	struct ParticipantDef
	{
		TilePosition corePosition;
		const Team* team = nullptr;
		ID core;
		IDs factoryBans;
		IDs abilityBans;
		std::array<ID, 6> factories;
		std::array<ID, 3> abilities;
		std::optional<ModPage> modifications;
		const ParticipantData* data = nullptr;
	};

	struct ParticipantInitializer
	{
		const ParticipantDef& participantInfo;
		GameDependencies& gameDependencies;
	};

	class PickParticipant
	{
	private:
		int m_Index = 0;

		const ParticipantData& m_Data;
		phase::Pick& m_PickPhase;
		const Team& m_Team;

		IDs m_AbilityPicks;
		IDs m_AbilityBans;
		IDs m_FactoryPicks;
		IDs m_FactoryBans;
		std::optional<ID> m_ModPageId;

	public:
		PickParticipant(int _index, const ParticipantData& _data, const Team& _team, phase::Pick& _pickState);

		int getIndex() const;
		const ParticipantData& getData() const;
		const Team& getTeam() const;

		void pickFactory(ID _id);
		void pickAbility(ID _id);
		void banFactory(ID _id);
		void banAbility(ID _id);
		void setModPage(ID _id);

		const IDs& getFactoryPicks() const;
		const IDs& getFactoryBans() const;
		const IDs& getAbilityPicks() const;
		const IDs& getAbilityBans() const;

		ParticipantDef generateDefinition() const;
		ID getColorID() const;
		std::optional<ID> getModPageId() const;

		void playEmote(ID _emoteId);
	};

	struct ParticipantStatistics
	{
		ParticipantData::TypeData typeData;

		ID teamId = 0;
		std::optional<ID> modPageId;
		IDs factoryBans;
		IDs abilityBans;

		Stat damageAmount = 0;
		Stat healAmount = 0;
		Stat coreMinHealth = 0;

		int minionSpawned = 0;
		int towerSpawned = 0;
		int wallSpawned = 0;

		int minionKilled = 0;
		int towerKilled = 0;
		int wallKilled = 0;

		struct Factory
		{
			ID id = 0;
			Stat xpGained = 0;
			device::FactoryUpgrades upgrades{};
		};
		std::array<Factory, MAX_FACTORIES> factories;

		struct Ability
		{
			ID id = 0;
			int uses = 0;
		};
		std::array<Ability, MAX_ABILITIES> abilities;
	};

	class Participant :
		public Castable
	{
	private:
		using super = Castable;

		int m_Index = 0;

		ParticipantDef m_ParticipantDef;

		bool m_HasLost = false;
		spell::TargetFinder m_TargetFinder;

		unit::UnitHandle m_GlobalTarget;
		ParticipantProgressFlags m_ProgressFlags;
		uint32_t m_InitWallsCounter = 0;
	
		// statistics
		std::array<uint32_t, static_cast<std::size_t>(unit::Type::max) - 1> m_KilledUnits{};
		std::array<uint32_t, static_cast<std::size_t>(unit::Type::max) - 1> m_SpawnedUnits{};
		Stat m_DamageDealt = 0;
		Stat m_Healt = 0;
	
		// maps
		map::VisibilityMap m_VisMap;
		map::BuildingMap m_BuildingMap;
		map::RoutingMap m_RoutingMap;
	
		// entities
		unit::Walls m_Walls;
		unit::Towers m_Towers;
		unit::Minions m_Minions;
		unit::Projectiles m_Projectiles;
		unit::Triggers m_Triggers;
		std::vector<std::unique_ptr<device::Device>> m_Devices;

		device::Abilities m_Abilities;
		device::Factories m_Factories;
	
		unit::Core& m_Core;		// define this member after all other members
	
		unit::Core& _setupCore(const ParticipantDef& _info);
		void _checkSettled();
		void _setupFactories(const ParticipantDef& _info);
		void _setupAbilities(const ParticipantDef& _info);
		void _updateMapData();
		void _recalculateMinionMovement();
		void _activateModifications(const ParticipantDef& _info);
		unit::Wall* _forceCreateWall(ID _id, const TilePosition& _pos);
	
		void _addMinion(unit::Unit& _unit);
		void _addWall(unit::Unit& _unit);
		void _addTower(unit::Unit& _unit);
		void _addProjectile(unit::Unit& _unit);
		void _addTrigger(unit::Unit& _unit);
		void _removeMinion(unit::Unit& _unit);
		void _removeWall(unit::Unit& _unit);
		void _removeTower(unit::Unit& _unit);
		void _removeProjectile(unit::Unit& _unit);
		void _removeTrigger(unit::Unit& _unit);
		std::optional<TileRect> _generateBuildRegionRestriction() const;

		std::vector<const unit::Unit*> _gatherVisAffectingUnits() const;
	
	public:
		Participant(int _index, const ParticipantInitializer& _initializer);

		int getIndex() const;
	
		void setupCastableSnapshot(CastableSnapshot& _info) const override;

		const ParticipantData& getData() const;
		const ParticipantDef& getDefinition() const;
	
		void start();
		void update(Time _diff);

		using Statistics = ParticipantStatistics;
		Statistics gatherStatistics() const;
		ID getColorID() const;
		const IDs& getSkins() const;
	
		void applyUnitKillToStatistics(unit::Type _type);
		void applyDamageDealtToStatistics(Stat _amount);
		void applyHealToStatistics(Stat _amount);
	
		void removeUnit(unit::Unit& _unit);
		void addUnit(unit::Unit& _unit);
	
		// nullptr if failed
		unit::Projectile* createProjectile(ID _id, const AbsPosition& _pos, ::game::spell::Spell& _spell);
		const unit::Projectiles& getProjectiles() const;
		// nullptr if failed
		unit::Trigger* createTrigger(ID _id, const AbsPosition& _pos);
		const unit::Triggers& getTriggers() const;
		// nullptr if failed
		unit::Minion* createMinion(ID _id, const AbsPosition& _pos, const TargetPreference* _preference = nullptr, Level* _xp = nullptr);
		const unit::Minions& getMinions() const;
		// nullptr if failed
		unit::Wall* createWall(ID _id, const TilePosition& _pos);
		const unit::Walls& getWalls() const;
		// nullptr if failed
		unit::Tower* createTower(ID _id, const TilePosition& _pos, unit::TowerTimer& _timer, const TargetPreference* _preference = nullptr, Level* _xp = nullptr);
		const unit::Towers& getTowers() const;

		unit::Core& getCore() const;
		bool hasLost() const;
		AbsPosition getCorePosition() const;
		const device::Factories& getFactories() const;
		const device::Abilities& getAbilities() const;
	
		const map::VisibilityMap& getVisibilityMap() const;
		map::BuildingMap& getBuildingMap();
		const map::BuildingMap& getBuildingMap() const;
		const std::vector<TilePosition>& getPossibleBuildPositions() const;
		const map::RoutingMap& getRoutingMap() const;
		Participant& getOpponent() const;
		const Team& getTeam() const;
		void setGlobalTarget(unit::GUID _guid);
		unit::UnitHandle getGlobalTarget() const;
		ParticipantProgressFlags getProgressFlags() const;

		void playEmote(ID _emoteId);
		void surrender();

		bool isParticipant() const final;
		bool isUnit() const final;
		const unit::Unit& toUnit() const final;
		unit::Unit& toUnit() final;
		bool isDevice() const final;
		const ::game::device::Device& toDevice() const final;
		::game::device::Device& toDevice() final;
	};
	using ParticipantPtr = std::unique_ptr<Participant>;

	template <class TUnits>
	int countLivingUnits(const TUnits& _units)
	{
		return std::count_if(std::begin(_units), std::end(_units),
			[](const auto _minion) { return _minion->isAlive(); }
		);
	}

	template <class TUnits>
	TUnits findLivingUnits(const TUnits& _units)
	{
		TUnits result;
		std::copy_if(std::begin(_units), std::end(_units), std::back_inserter(result),
			[](const auto _minion) { return _minion->isAlive(); }
		);
		return result;
	}

	float getMinionLimitInPercent(const Participant& _participant);

	class FinalizeParticipant
	{
	public:
		FinalizeParticipant(int _index, const ParticipantDef& _definition, phase::Finalize& _finalizePhase);


		int getIndex() const;
		const ParticipantDef& getDefinition() const;
		const Team& getTeam() const;

		void playEmote(ID _emoteId);

	private:
		int m_Index = 0;
		phase::Finalize& m_FinalizePhase;

		ParticipantDef m_ParticipantDef;
	};
	using FinalizeParticipantPtr = std::unique_ptr<FinalizeParticipant>;

	bool canPlayEmote(const PickParticipant& _participant, ID _emoteId);
	bool canPlayEmote(const Participant& _participant, ID _emoteId);
	bool canPlayEmote(const FinalizeParticipant& _participant, ID _emoteId);
	const unit::Minion* findForemostMinion(const Participant& _participant);

	bool isMemberOf(const Team& _team, int _index);

	template <class TTeams>
	const Team* findTeam(const TTeams& _teams, int _index)
	{
		assert(0 <= _index);
		auto itr = std::find_if(std::begin(_teams), std::end(_teams),
			[_index](auto& _team) { return isMemberOf(_team, _index); }
		);
		return itr != std::end(_teams) ? &*itr : nullptr;
	};

	template <class TParticipantContainer>
	bool isTeamDefeated(const Team& _team, const TParticipantContainer& _participants)
	{
		auto& indexes = _team.getMemberIndexes();
		return std::all_of(std::begin(indexes), std::end(indexes),
			[&_participants](int _index) { return _participants[_index]->hasLost(); }
		);
	}
} // namespace game
