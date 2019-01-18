#include "stdafx.h"
#include "Participant.hpp"
#include "Core.hpp"
#include "Wall.hpp"
#include "Tower.hpp"
#include "Minion.hpp"
#include "Projectile.hpp"
#include "Trigger.hpp"
#include "Factory.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"
#include "Game/Phase/PickPhase.hpp"
#include "Game/Phase/FinalizePhase.hpp"
#include "Data/Game.hpp"
#include "Data/Unit.hpp"
#include "Data/Ability.hpp"

namespace game
{
	auto _findModPage(const std::vector<ModPage>& _pages, ID _pid)
	{
		return std::find_if(std::begin(_pages), std::end(_pages),
			[_pid](const auto& _page)
			{
				return std::get<ID>(_page) == _pid;
			}
		);
	}

	/*#####
	# PickParticipant
	#####*/
	PickParticipant::PickParticipant(int _index, const ParticipantData& _data, const Team& _team, phase::Pick& _pickPhase) :
		m_Data(_data),
		m_Team(_team),
		m_PickPhase(_pickPhase),
		m_Index(_index),
		m_ModPageId(_data.getLastModPageId())
	{
		assert(0 <= m_Index);
	}

	int PickParticipant::getIndex() const
	{
		return m_Index;
	}

	const ParticipantData& PickParticipant::getData() const
	{
		return m_Data;
	}

	const Team& PickParticipant::getTeam() const
	{
		return m_Team;
	}

	void PickParticipant::pickFactory(ID _id)
	{
		assert(_id > 0);
		m_FactoryPicks.emplace_back(_id);
		LOG_INFO("Participant id: " << getIndex() << " picked factory id: " << _id);
	}

	void PickParticipant::pickAbility(ID _id)
	{
		assert(_id > 0);
		m_AbilityPicks.emplace_back(_id);
		LOG_INFO("Participant id: " << getIndex() << " picked ability id: " << _id);
	}

	void PickParticipant::banFactory(ID _id)
	{
		assert(_id > 0);
		m_FactoryBans.emplace_back(_id);
		LOG_INFO("Participant id: " << getIndex() << " banned factory id: " << _id);
	}

	void PickParticipant::banAbility(ID _id)
	{
		assert(_id > 0);
		m_AbilityBans.emplace_back(_id);
		LOG_INFO("Participant id: " << getIndex() << " banned ability id: " << _id);
	}

	void PickParticipant::setModPage(ID _id)
	{
		if (_findModPage(m_Data.getModPages(), _id) != std::end(m_Data.getModPages()))
		{
			m_ModPageId = _id;
			LOG_INFO("Participant id: " << getIndex() << " switched to mod page id: " << _id);
		}
		else
			LOG_ERR("Participant id: " << getIndex() << " failed switched to mod page id: " << _id);
	}

	const IDs& PickParticipant::getFactoryPicks() const
	{
		return m_FactoryPicks;
	}

	const IDs& PickParticipant::getFactoryBans() const
	{
		return m_FactoryBans;
	}

	const IDs& PickParticipant::getAbilityPicks() const
	{
		return m_AbilityPicks;
	}

	const IDs& PickParticipant::getAbilityBans() const
	{
		return m_AbilityBans;
	}

	ParticipantDef PickParticipant::generateDefinition() const
	{
		ParticipantDef def;
		def.core = 1;
		def.factories[0] = 1;
		def.factoryBans = m_FactoryBans;
		def.abilityBans = m_AbilityBans;
		assert(m_FactoryPicks.size() == def.factories.size() - 1);
		std::copy(std::begin(m_FactoryPicks), std::end(m_FactoryPicks), ++std::begin(def.factories));
		assert(m_AbilityPicks.size() == def.abilities.size());
		std::copy(std::begin(m_AbilityPicks), std::end(m_AbilityPicks), std::begin(def.abilities));
		if (auto modPageId = getModPageId())
		{
			auto itr = _findModPage(m_Data.getModPages(), *modPageId);
			assert(itr != std::end(m_Data.getModPages()));
			def.modifications = *itr;
		}
		def.data = &m_Data;
		return def;
	}

	ID PickParticipant::getColorID() const
	{
		return m_Data.getColorId();
	}

	std::optional<ID> PickParticipant::getModPageId() const
	{
		return m_ModPageId;
	}

	void PickParticipant::playEmote(ID _emoteId)
	{
		if (canPlayEmote(*this, _emoteId))
			m_PickPhase.getVisualCollector().addEmote(m_Index, _emoteId);
	}

	/*#####
	# Participant
	#####*/
	unit::Minion* _findForemostMinion(const unit::Minions& _minions, const Participant& _opponent)
	{
		auto& routingMap = _opponent.getRoutingMap();

		auto tileCost = std::numeric_limits<int>::max();
		auto costToNextTile = std::numeric_limits<int>::max();
		unit::Minion* foremostMinion = nullptr;
		for (auto ptr : _minions)
		{
			assert(ptr);
			auto& minion = *ptr;
			if (minion.isDead())
				continue;
			if (auto destination = minion.getDestination())
			{
				auto curTileCost = routingMap.get(mapToTilePosition(*destination));
				auto curCostToNextTile = minion.getDistanceToDestination();
				if (curCostToNextTile && (curTileCost < tileCost || curTileCost == tileCost && *curCostToNextTile < costToNextTile))
				{
					tileCost = curTileCost;
					costToNextTile = *curCostToNextTile;
					foremostMinion = ptr;
				}
			}
		}
		return foremostMinion;
	}

	Participant::Participant(int _index, const ParticipantInitializer& _initializer) :
		super({ _initializer.gameDependencies, m_TargetFinder }),
		m_BuildingMap(*this),
		m_Core(_setupCore(_initializer.participantInfo)),
		m_RoutingMap(_initializer.gameDependencies.tileMap),
		m_TargetFinder(*this),
		m_ParticipantDef(_initializer.participantInfo),
		m_VisMap(_initializer.gameDependencies.tileMap.getSize()),
		m_Index(_index)
	{
		assert(0 <= m_Index);

		_setupFactories(_initializer.participantInfo);
		assert(std::none_of(std::begin(m_Factories), std::end(m_Factories), [](device::Factory* _factory) {
			return _factory == nullptr;
		}));

		_setupAbilities(_initializer.participantInfo);
		assert(std::none_of(std::begin(m_Abilities), std::end(m_Abilities), [](device::Ability* _ability) {
			return _ability == nullptr;
		}));

		_activateModifications(_initializer.participantInfo);

		m_KilledUnits.fill(0);
		m_SpawnedUnits.fill(0);
	}

	int Participant::getIndex() const
	{
		return m_Index;
	}

	void Participant::start()
	{
#ifndef NDEBUG
		auto index = getGameDependencies().gamePhase.getParticipants().at(0).get() == this ? 0 : 1;

		if (auto& debug = getGameDependencies().gameData.gameProperties.game.debug)
		{
			for (auto& pos : debug->players.at(index).wallLocations)
				_forceCreateWall(1, pos);
		}
#endif
		_updateMapData();

		// setup initial walls
		m_InitWallsCounter = *getGameDependencies().gameData.gameProperties.game.initialWalls;
		_checkSettled();
		if (auto& factory = *m_Factories[0];
			factory.getType() == device::FactoryType::wall)
		{
			static_cast<device::WallFactory&>(factory).setStack(m_InitWallsCounter);
		}

		for (auto& ability : m_Abilities)
			ability->startPlay();
		for (auto& factory : m_Factories)
			factory->startPlay();
	}

	unit::Core& Participant::_setupCore(const ParticipantDef& _info)
	{
		auto& gameData = getGameDependencies().gameData;
		auto& tileMap = getGameDependencies().tileMap;
		if (tileMap.get(_info.corePosition))
			throw std::runtime_error("Participant: Core tile already blocked - x: " + std::to_string(_info.corePosition.getX()) + " y: " + std::to_string(_info.corePosition.getY()));
		// lookup core info
		auto coreData = data::findData(gameData.cores, _info.core);
		if (!coreData)
			throw std::runtime_error("Participant: Core ID: " + std::to_string(_info.core) + " not found.");
		unit::CoreInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, nullptr, *coreData }, *coreData, _info.corePosition };
		auto& core = getGameDependencies().world.createUnit<unit::Core>(init);
		core.addIntoWorld();
		tileMap.set(core);
		return core;
	}

	void Participant::_checkSettled()
	{
		if (m_InitWallsCounter == 0)
			m_ProgressFlags.apply(ParticipantProgress::settled);
	}

	void Participant::_setupFactories(const ParticipantDef& _info)
	{
		assert(m_Factories.size() == _info.factories.size());

		auto& gameData = getGameDependencies().gameData;
		for (std::size_t i = 0; i < m_Factories.size(); ++i)
		{
			auto factoryData = data::findData(gameData.factories, _info.factories[i]);
			if (!factoryData)
				throw std::runtime_error("Participant: Factory ID: " + std::to_string(_info.factories[i]) + " not found.");
			device::FactoryInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, *factoryData }, *factoryData };
			auto factory = device::createFactory(init);
			m_Factories.at(i) = factory.get();
			m_Devices.emplace_back(std::move(factory));
		}
	}

	void Participant::_setupAbilities(const ParticipantDef& _info)
	{
		auto& gameData = getGameDependencies().gameData;
		for (std::size_t i = 0; i < m_Abilities.size(); ++i)
		{
			auto abilityData = data::findData(gameData.abilities, _info.abilities[i]);
			if (!abilityData)
				throw std::runtime_error("Participant: Ability ID: " + std::to_string(_info.abilities[i]) + " not found.");
			device::AbilityInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, *abilityData }, *abilityData };
			auto ability = std::make_unique<device::Ability>(init);
			m_Abilities.at(i) = ability.get();
			m_Devices.emplace_back(std::move(ability));
		}
	}

	void Participant::_updateMapData()
	{
		auto start = std::chrono::steady_clock::now();
		getGameDependencies().pathMap.renew();

		m_RoutingMap.generate(mapToTilePosition(getCorePosition()));
		m_BuildingMap.renewStaticMap(_generateBuildRegionRestriction());

		auto& opponent = getOpponent();
		opponent.m_RoutingMap.generate(mapToTilePosition(opponent.getCorePosition()));
		opponent.m_BuildingMap.renewStaticMap(opponent._generateBuildRegionRestriction());

		LOG_DEBUG("Participant: map data rebuild in " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start) << ".");
	}

	void Participant::_recalculateMinionMovement()
	{
		for (auto minion : m_Minions)
		{
			assert(minion);
			minion->recalculateMovement();
		}
	}

	void Participant::_activateModifications(const ParticipantDef& _info)
	{
		if (!_info.modifications)
			return;
		auto& spellDB = getGameDependencies().gameData.spells;
		auto& modDB = getGameDependencies().gameData.modifications;
		for (auto modId : std::get<IDs>(*_info.modifications))
		{
			if (auto mod = data::findData(modDB, modId))
			{
				if (auto spell = data::findData(spellDB, *mod->spellId))
					cast(*spell, getCastableHandle());
			}
		}
	}

	template <class TContainer, class TUnit>
	void _insertIntoContainer(TContainer& _container, TUnit& _unit)
	{
		auto itr = std::lower_bound(std::rbegin(_container), std::rend(_container), _unit.getGUID(),
			[](auto& _unit, auto _guid) { return _guid < _unit->getGUID(); }
		);
		assert(itr == std::rend(_container) || (*itr)->getGUID() != _unit.getGUID());
		_container.emplace(itr.base(), &_unit);
	}

	void Participant::_addMinion(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::minion);
		_insertIntoContainer(m_Minions, static_cast<unit::Minion&>(_unit));
	}

	void Participant::_addWall(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::wall);
		_insertIntoContainer(m_Walls, static_cast<unit::Wall&>(_unit));

		auto& opponent = getOpponent();
		_updateMapData();
		_recalculateMinionMovement();
		opponent._recalculateMinionMovement();
	}

	void Participant::_addTower(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::tower);
		_insertIntoContainer(m_Towers, static_cast<unit::Tower&>(_unit));
	}

	void Participant::_addProjectile(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::projectile);
		_insertIntoContainer(m_Projectiles, static_cast<unit::Projectile&>(_unit));
	}

	void Participant::_addTrigger(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::trigger);
		_insertIntoContainer(m_Triggers, static_cast<unit::Trigger&>(_unit));
	}

	void Participant::_removeMinion(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::minion);
		auto itr = std::find(std::begin(m_Minions), std::end(m_Minions), &_unit);
		assert(itr != std::end(m_Minions));
		m_Minions.erase(itr);
	}

	void Participant::_removeWall(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::wall);
		auto itr = std::find(std::begin(m_Walls), std::end(m_Walls), &_unit);
		assert(itr != std::end(m_Walls));
		m_Walls.erase(itr);

		auto& opponent = getOpponent();
		_updateMapData();
		_recalculateMinionMovement();
		opponent._recalculateMinionMovement();
	}

	void Participant::_removeTower(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::tower);
		auto itr = std::find(std::begin(m_Towers), std::end(m_Towers), &_unit);
		assert(itr != std::end(m_Towers));
		m_Towers.erase(itr);
	}

	void Participant::_removeProjectile(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::projectile);
		auto itr = std::find(std::begin(m_Projectiles), std::end(m_Projectiles), &_unit);
		assert(itr != std::end(m_Projectiles));
		m_Projectiles.erase(itr);
	}

	void Participant::_removeTrigger(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this && _unit.getType() == unit::Type::trigger);
		auto itr = std::find(std::begin(m_Triggers), std::end(m_Triggers), &_unit);
		assert(itr != std::end(m_Triggers));
		m_Triggers.erase(itr);
	}

	std::optional<TileRect> Participant::_generateBuildRegionRestriction() const
	{
		if (!m_ProgressFlags.contains(ParticipantProgress::settled))
		{
			auto coreTile = mapToTilePosition(getCorePosition());
			return TileRect(coreTile.getX() == 0 ? 0 : 8, 0, 7, 7);
		}
		return {};
	}

	std::vector<const unit::Unit*> Participant::_gatherVisAffectingUnits() const
	{
		std::vector<const unit::Unit*> units;
		units.reserve(m_Walls.size() + m_Towers.size() + m_Minions.size() + m_Triggers.size() + m_Projectiles.size() + 1/*core*/);
		auto pred = [](auto _unit) {
			assert(_unit);
			return !_unit->hasUnitFlag(unit::Flags::noVisibilityModification);
		};
		std::copy_if(std::begin(m_Walls), std::end(m_Walls), std::back_inserter(units), pred);
		std::copy_if(std::begin(m_Towers), std::end(m_Towers), std::back_inserter(units), pred);
		std::copy_if(std::begin(m_Minions), std::end(m_Minions), std::back_inserter(units), pred);
		std::copy_if(std::begin(m_Triggers), std::end(m_Triggers), std::back_inserter(units), pred);
		std::copy_if(std::begin(m_Projectiles), std::end(m_Projectiles), std::back_inserter(units), pred);
		if (pred(&m_Core))
			units.emplace_back(&m_Core);
		return units;
	}

	void _updateFactories(device::Factories& _factories, Time _diff)
	{
		for (auto& factory : _factories)
		{
			assert(factory);
			factory->update(_diff);
		}
	}

	void _updateAbilities(device::Abilities& _abilities, Time _diff)
	{
		for (auto& ability : _abilities)
		{
			assert(ability);
			ability->update(_diff);
		}
	}

	// don't pass dead minions
	std::vector<unit::Minion*> _findForemosteMinions(unit::Minions& _minions, int _minionLimit, const map::RoutingMap& _routingMapt)
	{
		assert(0 <= _minionLimit && _minionLimit < std::size(_minions));
		auto lessCompareDistance = [&_routingMapt](const unit::Minion* _lhs, const unit::Minion* _rhs)
		{
			assert(_lhs && _rhs && _lhs->isAlive() && _rhs->isAlive());
			auto calculateTotalDistance = [&_routingMapt](const unit::Minion* _lhs) -> std::optional<AbsCoordType>
			{
				if (auto destination = _lhs->getDestination())
				{
					if (auto curCostToNextTile = _lhs->getDistanceToDestination())
						return *curCostToNextTile + _routingMapt.get(mapToTilePosition(*destination));
				}
				return std::nullopt;
			};
			auto lhsDist = calculateTotalDistance(_lhs);
			auto rhsDist = calculateTotalDistance(_rhs);
			return lhsDist.value_or(1000) < rhsDist.value_or(1000);
		};
		std::vector<unit::Minion*> aboutToDie(std::size(_minions) - _minionLimit);
		std::partial_sort_copy(std::begin(_minions), std::end(_minions), std::begin(aboutToDie), std::end(aboutToDie), lessCompareDistance);
		return aboutToDie;
	}

	void _checkMinionLimit(Participant& _participant, const data::Game& _gameData)
	{
		if (auto livingUnits = findLivingUnits(_participant.getMinions()); _gameData.gameProperties.game.minionLimit < std::size(livingUnits))
		{
			auto spellId = _gameData.gameProperties.game.minionLimitExplodeSpellID;
			auto& spellTable = _gameData.spells;
			if (auto spell = data::findData(spellTable, *spellId))
			{
				for (auto minion : _findForemosteMinions(livingUnits, *_gameData.gameProperties.game.minionLimit, _participant.getOpponent().getRoutingMap()))
				{
					assert(minion && minion->isAlive());
					minion->cast(*spell, minion->getCastableHandle());
					minion->kill();
				}
			}
		}
	}

	void Participant::update(Time _diff)
	{
		if (isRunning(getGameDependencies().gamePhase))
		{
			_updateFactories(m_Factories, _diff);
			_updateAbilities(m_Abilities, _diff);
			_checkMinionLimit(*this, getGameDependencies().gameData);
			if (m_Core.isDead())
				m_HasLost = true;
		}

		m_VisMap.renewDynamicMap(_gatherVisAffectingUnits());
		m_BuildingMap.renew(_generateBuildRegionRestriction());
	}

	Participant::Statistics Participant::gatherStatistics() const
	{
		Participant::Statistics stats;

		stats.typeData = m_ParticipantDef.data->getTypeData();
		if (m_ParticipantDef.modifications)
			stats.modPageId = std::get<ID>(*m_ParticipantDef.modifications);
		stats.teamId = getTeam().getId();

		// pick infos
		stats.factoryBans = m_ParticipantDef.factoryBans;
		stats.abilityBans = m_ParticipantDef.abilityBans;

		// game infos
		auto& coreState = std::get<unit::CoreState>(m_Core.getUnitState().derivedState);
		stats.coreMinHealth = coreState.minHpPercent;
		stats.damageAmount = m_DamageDealt;
		stats.healAmount = m_Healt;

		stats.minionKilled = m_KilledUnits[static_cast<std::size_t>(unit::Type::minion) - 1];
		stats.wallKilled = m_KilledUnits[static_cast<std::size_t>(unit::Type::wall) - 1];
		stats.towerKilled = m_KilledUnits[static_cast<std::size_t>(unit::Type::tower) - 1];

		stats.minionSpawned = m_SpawnedUnits[static_cast<std::size_t>(unit::Type::minion) - 1];
		stats.wallSpawned = m_SpawnedUnits[static_cast<std::size_t>(unit::Type::wall) - 1];
		stats.towerSpawned = m_SpawnedUnits[static_cast<std::size_t>(unit::Type::tower) - 1];

		assert(m_Factories.size() == stats.factories.size());
		for (std::size_t i = 0; i < m_Factories.size(); ++i)
		{
			auto& out = m_Factories[i];
			auto& in = stats.factories[i];
			in.id = *out->getFactoryData().id;
			in.xpGained = out->getXPTotal();
			in.upgrades = out->getUpgrades();
		}

		assert(m_Abilities.size() == stats.abilities.size());
		for (std::size_t i = 0; i < m_Abilities.size(); ++i)
		{
			auto& out = m_Abilities[i];
			auto& in = stats.abilities[i];
			in.id = *out->getAbilityData().id;
			in.uses = out->getUses();
		}
		return stats;
	}

	ID Participant::getColorID() const
	{
		assert(m_ParticipantDef.data);
		return m_ParticipantDef.data->getColorId();
	}

	const IDs& Participant::getSkins() const
	{
		assert(m_ParticipantDef.data);
		return m_ParticipantDef.data->getSkins();
	}

	void Participant::applyUnitKillToStatistics(unit::Type _type)
	{
		++m_KilledUnits.at(static_cast<std::size_t>(_type) - 1);
	}

	void Participant::applyDamageDealtToStatistics(Stat _amount)
	{
		m_DamageDealt += _amount;
	}

	void Participant::applyHealToStatistics(Stat _amount)
	{
		m_Healt += _amount;
	}

	void Participant::removeUnit(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this);
		using unit::Type;
		switch (_unit.getType())
		{
		case Type::core: break;
		case Type::minion: _removeMinion(_unit); break;
		case Type::wall: _removeWall(_unit); break;
		case Type::tower: _removeTower(_unit); break;
		case Type::projectile: _removeProjectile(_unit); break;
		case Type::trigger: _removeTrigger(_unit); break;
		}
	}

	void Participant::addUnit(unit::Unit& _unit)
	{
		assert(&_unit.getOwner() == this);
		using unit::Type;
		switch (_unit.getType())
		{
		case Type::core: break;
		case Type::minion: _addMinion(_unit); break;
		case Type::wall: _addWall(_unit); break;
		case Type::tower: _addTower(_unit); break;
		case Type::projectile: _addProjectile(_unit); break;
		case Type::trigger: _addTrigger(_unit); break;
		}
	}

	unit::Projectile* Participant::createProjectile(ID _id, const AbsPosition& _pos, spell::Spell& _spell)
	{
		if (auto data = data::findData(getGameDependencies().gameData.projectiles, _id))
		{
			unit::ProjectileInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, nullptr, *data }, _spell, _pos, *data };
			auto& projectile = getGameDependencies().world.createUnit<unit::Projectile>(init);
			m_Projectiles.push_back(&projectile);
			return &projectile;
		}
		else
			LOG_ERR("Participant: invalid projectile id: " + _id);
		return nullptr;
	}

	const unit::Projectiles& Participant::getProjectiles() const
	{
		return m_Projectiles;
	}

	unit::Trigger* Participant::createTrigger(ID _id, const AbsPosition& _pos)
	{
		if (auto data = data::findData(getGameDependencies().gameData.triggers, _id))
		{
			unit::TriggerInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, nullptr, *data }, *data };
			auto& trigger = getGameDependencies().world.createUnit<unit::Trigger>(init);
			m_Triggers.push_back(&trigger);
			trigger.setPosition(_pos);
			trigger.addIntoWorld();
			return &trigger;
		}
		else
			LOG_ERR("Participant: invalid trigger id: " + _id);
		return nullptr;
	}

	const unit::Triggers& Participant::getTriggers() const
	{
		return m_Triggers;
	}

	unit::Minion* Participant::createMinion(ID _id, const AbsPosition& _pos, const TargetPreference* _preference, Level* _xp)
	{
		auto data = data::findData(getGameDependencies().gameData.minions, _id);
		if (!data)
		{
			LOG_ERR("Participant: invalid minion id: " + _id);
			return nullptr;
		}

		auto tileUnit = getGameDependencies().tileMap.get(mapToTilePosition(_pos));
		if (tileUnit && !tileUnit->hasUnitFlag(unit::Flags::unsolid))
			return nullptr;

		unit::MinionInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, _xp, *data }, *data, _preference };
		auto& minion = getGameDependencies().world.createUnit<unit::Minion>(init);
		_addMinion(minion);
		minion.setPosition(_pos);
		minion.addIntoWorld();
		minion.setMovementGenerator(std::make_unique<unit::PathMovementGenerator>(minion, getOpponent().getRoutingMap()));

		++m_SpawnedUnits[static_cast<std::size_t>(unit::Type::minion) - 1];
		return &minion;
	}

	const unit::Minions& Participant::getMinions() const
	{
		return m_Minions;
	}

	unit::Wall* Participant::_forceCreateWall(ID _id, const TilePosition& _pos)
	{
		auto wallData = data::findData(getGameDependencies().gameData.walls, _id);
		if (!wallData)
		{
			LOG_ERR("Participant: invalid wall id: " + _id);
			return nullptr;
		}

		assert(getGameDependencies().tileMap.get(_pos) == nullptr);

		unit::WallInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, nullptr, *wallData }, *wallData, _pos };
		auto& wall = getGameDependencies().world.createUnit<unit::Wall>(init);
		m_Walls.push_back(&wall);
		wall.addIntoWorld();
		return &wall;
	}

	unit::Wall* Participant::createWall(ID _id, const TilePosition& _pos)
	{
		if (!m_BuildingMap.getBuildingMap().isSet(_pos))
		{
			LOG_ERR("Participant: Wall building at x: " << _pos.getX() << " y: " << _pos.getY() << " not possible. No valid position.");
			return nullptr;
		}

		if (auto wall = _forceCreateWall(_id, _pos))
		{
			if (!m_ProgressFlags.contains(ParticipantProgress::settled))
			{
				--m_InitWallsCounter;
				_checkSettled();
			}
			_updateMapData();
			auto& opponent = getOpponent();
			_recalculateMinionMovement();
			opponent._recalculateMinionMovement();

			++m_SpawnedUnits[static_cast<std::size_t>(unit::Type::wall) - 1];
			return wall;
		}
		return nullptr;
	}

	const unit::Walls& Participant::getWalls() const
	{
		return m_Walls;
	}

	unit::Tower* Participant::createTower(ID _id, const TilePosition& _pos, unit::TowerTimer& _timer, const TargetPreference* _preference, Level* _xp)
	{
		auto towerData = data::findData(getGameDependencies().gameData.towers, _id);
		if (!towerData)
		{
			LOG_ERR("Participant: invalid tower id: " + _id);
			return nullptr;
		}
		if (!unit::Tower::isValidPosition(getGameDependencies().tileMap, _pos, *this))
		{
			LOG_ERR("Participant: unable to create tower at " << _pos.getX() << "/" << _pos.getY() << ". Position is already occupied.");
			return nullptr;
		}

		unit::TowerInitializer init{ { { { getGameDependencies(), m_TargetFinder }, *this }, _xp, *towerData }, *towerData, _preference, _timer };
		auto& tower = getGameDependencies().world.createUnit<unit::Tower>(init);
		tower.setPosition(mapToAbsPosition(_pos));
		m_Towers.emplace_back(&tower);

		++m_SpawnedUnits[static_cast<std::size_t>(unit::Type::tower) - 1];
		return &tower;
	}

	const unit::Towers& Participant::getTowers() const
	{
		return m_Towers;
	}

	unit::Core& Participant::getCore() const
	{
		return m_Core;
	}

	bool Participant::hasLost() const
	{
		return m_HasLost;
	}

	AbsPosition Participant::getCorePosition() const
	{
		return m_Core.getPosition();
	}

	const device::Factories& Participant::getFactories() const
	{
		return m_Factories;
	}

	const device::Abilities& Participant::getAbilities() const
	{
		return m_Abilities;
	}

	const map::VisibilityMap& Participant::getVisibilityMap() const
	{
		return m_VisMap;
	}

	map::BuildingMap& Participant::getBuildingMap()
	{
		return m_BuildingMap;
	}

	const map::BuildingMap& Participant::getBuildingMap() const
	{
		return m_BuildingMap;
	}

	const std::vector<TilePosition>& Participant::getPossibleBuildPositions() const
	{
		return m_BuildingMap.getPossibleBuildPositions();
	}

	const map::RoutingMap& Participant::getRoutingMap() const
	{
		return m_RoutingMap;
	}

	Participant& Participant::getOpponent() const
	{
		// ToDo: add team support
		auto& gameState = getGameDependencies().gamePhase;
		return gameState.getParticipants().at(0).get() != this ? *gameState.getParticipants().at(0) : *gameState.getParticipants().at(1);
	}

	const Team& Participant::getTeam() const
	{
		assert(m_ParticipantDef.team);
		return *m_ParticipantDef.team;
	}

	void Participant::setGlobalTarget(unit::GUID _guid)
	{
		if (_guid == 0)
		{
			m_GlobalTarget.reset();
			LOG_DEBUG("Participant cleared global target.");
		}
		else if (auto unitPtr = getGameDependencies().world.findUnit(_guid))
		{
			if (unitPtr->isTargetable() && unit::isHostile(*unitPtr, *this))
			{
				m_GlobalTarget = unitPtr->getUnitHandle();
				LOG_DEBUG("Participant set global target GUID: " + unitPtr->getGUID());
			}
		}
	}

	unit::UnitHandle Participant::getGlobalTarget() const
	{
		if (m_GlobalTarget && isAlive(*m_GlobalTarget) && isInWorld(*m_GlobalTarget))
			return m_GlobalTarget;
		return unit::UnitHandle();
	}

	ParticipantProgressFlags Participant::getProgressFlags() const
	{
		return m_ProgressFlags;
	}

	void Participant::playEmote(ID _emoteId)
	{
		if (canPlayEmote(*this, _emoteId))
			getGameDependencies().gamePhase.getVisualCollector().addEmote(m_Index, _emoteId);
	}

	void Participant::surrender()
	{
		m_HasLost = true;
	}

	void Participant::setupCastableSnapshot(CastableSnapshot& _info) const
	{
		super::setupCastableSnapshot(_info);

		_info.participant = const_cast<Participant*>(this);
	}

	const ParticipantData& Participant::getData() const
	{
		assert(m_ParticipantDef.data);
		return *m_ParticipantDef.data;
	}

	const ParticipantDef& Participant::getDefinition() const
	{
		return m_ParticipantDef;
	}

	bool Participant::isParticipant() const
	{
		return true;
	}

	bool Participant::isDevice() const
	{
		return false;
	}

	const device::Device& Participant::toDevice() const
	{
		throw std::runtime_error("Castable: Is no Device type.");
	}

	device::Device& Participant::toDevice()
	{
		throw std::runtime_error("Castable: Is no Device type.");
	}

	bool Participant::isUnit() const
	{
		return false;
	}

	const unit::Unit& Participant::toUnit() const
	{
		throw std::runtime_error("Castable: Is no Unit type.");
	}

	unit::Unit& Participant::toUnit()
	{
		throw std::runtime_error("Castable: Is no Unit type.");
	}

	float getMinionLimitInPercent(const Participant& _participant)
	{
		return countLivingUnits(_participant.getMinions()) * 100.f / *_participant.getGameDependencies().gameData.gameProperties.game.minionLimit;
	}

	bool _canPlayEmote(const IDs& _emotes, ID _emoteId)
	{
		assert(0 < _emoteId);
		return std::binary_search(std::begin(_emotes), std::end(_emotes), _emoteId);
	}

	bool canPlayEmote(const PickParticipant& _participant, ID _emoteId)
	{
		return _canPlayEmote(_participant.getData().getUnlocks(UnlockType::emote), _emoteId);
	}

	bool canPlayEmote(const Participant& _participant, ID _emoteId)
	{
		return _canPlayEmote(_participant.getData().getUnlocks(UnlockType::emote), _emoteId);
	}

	bool canPlayEmote(const FinalizeParticipant& _participant, ID _emoteId)
	{
		assert(_participant.getDefinition().data);
		return _canPlayEmote(_participant.getDefinition().data->getUnlocks(UnlockType::emote), _emoteId);
	}

	const unit::Minion* findForemostMinion(const Participant& _participant)
	{
		return _findForemostMinion(_participant.getMinions(), _participant.getOpponent());
	}

	FinalizeParticipant::FinalizeParticipant(int _index, const ParticipantDef& _definition, phase::Finalize& _finalizePhase) :
		m_Index(_index),
		m_ParticipantDef(_definition),
		m_FinalizePhase(_finalizePhase)
	{
		assert(m_ParticipantDef.data);
	}

	int FinalizeParticipant::getIndex() const
	{
		assert(0 <= m_Index);
		return m_Index;
	}

	const ParticipantDef& FinalizeParticipant::getDefinition() const
	{
		return m_ParticipantDef;
	}

	const Team& FinalizeParticipant::getTeam() const
	{
		assert(m_ParticipantDef.team);
		return *m_ParticipantDef.team;
	}

	void FinalizeParticipant::playEmote(ID _emoteId)
	{
		if (canPlayEmote(*this, _emoteId))
			m_FinalizePhase.getVisualCollector().addEmote(m_Index, _emoteId);
	}

	bool isMemberOf(const Team& _team, int _index)
	{
		return std::binary_search(std::begin(_team.getMemberIndexes()), std::end(_team.getMemberIndexes()), _index);
	}
} // namespace game
