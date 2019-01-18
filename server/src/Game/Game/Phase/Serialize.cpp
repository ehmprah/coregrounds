#include "stdafx.h"
#include "Serialize.hpp"
#include "AbstractGamePhase.hpp"
#include "PickPhase.hpp"
#include "FinalizePhase.hpp"
#include "Map/VisibilityMap.hpp"
#include "Game/Entity/Participant.hpp"
#include "Game/Entity/Core.hpp"
#include "Game/Entity/Minion.hpp"
#include "Game/Entity/Tower.hpp"
#include "Game/Entity/Wall.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Entity/Trigger.hpp"

template <class Container>
protobuf::out::Match::OptionalIds _serializeOptionalIds(const Container& _container)
{
	protobuf::out::Match::OptionalIds ids;
	ids.set_isset(true);
	for (auto id : _container)
		ids.add_ids(id);
	return ids;
}

namespace game::phase_pick
{
	using ProtoPick = protobuf::out::Match::Pick;

	using Players = google::protobuf::RepeatedPtrField<ProtoPick::Player>;
	void _serializePickPlayers(Players& _players, const phase::Pick& _phase, const std::optional<int>& _currentParticipant)
	{
		for (auto& participant : _phase.getParticipants())
		{
			auto& playerMsg = *_players.Add();
			playerMsg.set_colorid(setHighestBit(participant->getColorID()));
			playerMsg.set_teamid(participant->getTeam().getId());

			if (auto& emote = _phase.getVisualCollector().getEmote(participant->getIndex()))
				playerMsg.set_emoteid(*emote);

			for (auto id : participant->getFactoryBans())
				playerMsg.add_factorybans(id);
			for (auto id : participant->getAbilityBans())
				playerMsg.add_abilitybans(id);
			for (auto id : participant->getFactoryPicks())
				playerMsg.add_factorypicks(id);
			for (auto id : participant->getAbilityPicks())
				playerMsg.add_abilitypicks(id);

			auto& skins = participant->getData().getSkins();
			if (!skins.empty())
				*playerMsg.mutable_skins() = _serializeOptionalIds(skins);

			if (!_currentParticipant || *_currentParticipant == participant->getIndex())
			{
				if (participant->getModPageId())
					playerMsg.set_modpageid(*participant->getModPageId());

				using PickState = phase::Pick::State;
				switch (_phase.getState())
				{
				case PickState::banFactory:
					for (auto id : _phase.getBanableFactories(*participant))
						playerMsg.add_availableids(id);
					break;
				case PickState::pickFactory:
					for (auto id : _phase.getPickableFactories(*participant))
						playerMsg.add_availableids(id);
					break;
				case PickState::pickAbility:
					for (auto id : _phase.getPickableAbilities(*participant))
						playerMsg.add_availableids(id);
					break;
				case PickState::banAbility:
					for (auto id : _phase.getBanableAbilities(*participant))
						playerMsg.add_availableids(id);
					break;
				default:
					break;
				}
			}
		}
	}
} // game::phase_pick

namespace game::phase_game
{
	using ProtoGame =  protobuf::out::Match::Game;

	std::uint32_t _serializeDirectionVector(DirectionVector _dirVector)
	{
		_dirVector += 1;
		_dirVector *= 1000;
		return static_cast<std::uint32_t>(_dirVector.getX()) | (static_cast<std::uint32_t>(_dirVector.getY()) << 15);
	}

	std::uint32_t _serializePosition(AbsPosition _pos)
	{
		return std::max<std::uint32_t>(0, _pos.getX() * 1000) | (std::max<std::uint32_t>(0, _pos.getY() * 1000) << 15);
	}

	template <class Container>
	auto _selectVisibleUnits(const Container& _container, const map::VisibilityMap& _visMap)
	{
		Container result;
		result.reserve(std::size(_container));
		std::copy_if(std::begin(_container), std::end(_container), std::back_inserter(result),
			[&_visMap](auto _unit) { return _visMap.isVisible(*_unit); }
		);
		return result;
	}

	void _serializeUnit(const unit::Unit& _unit, ProtoGame::Unit& _msg)
	{
		auto& unitState = _unit.getUnitState();
		_msg.set_guid(_unit.getGUID());
		_msg.set_position(setHighestBit(_serializePosition(unitState.pos)));
		_msg.set_hp(setHighestBit(static_cast<std::uint32_t>(getHealthPercent(unitState))));
		_msg.set_id(setHighestBit(_unit.getId()));

#ifndef NDEBUG
		for (std::size_t i = static_cast<std::size_t>(unit::StatType::none) + 1; i < static_cast<std::size_t>(unit::StatType::max); ++i)
			_msg.add_stats(unitState.stats.getStat(static_cast<unit::StatType>(i)));
#endif

		auto& visIdMsg = *_msg.mutable_auravisualids();
		visIdMsg.set_isset(true);
		for (auto id : unitState.visualIds)
			visIdMsg.add_ids(id);

		_msg.set_state(setHighestBit(static_cast<std::uint32_t>(unitState.state)));
	}

	void _serializeUnit(const unit::Core& _unit, ProtoGame::Core& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());
	}

	void _serializeUnit(const unit::Wall& _unit, ProtoGame::Wall& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());

		auto& derivedState = std::get<unit::WallState>(_unit.getUnitState().derivedState);
		_msg.set_chainbonus(setHighestBit(derivedState.chainBonus));
	}

	void _serializeUnit(const unit::Minion& _unit, ProtoGame::Minion& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());

		auto& derivedState = std::get<unit::MinionState>(_unit.getUnitState().derivedState);
		_msg.set_attackprogress(setHighestBit(static_cast<std::uint32_t>(derivedState.attackProgressPercent)));
		_msg.set_directionvector(setHighestBit(_serializeDirectionVector(derivedState.directionVector)));
	}

	void _serializeUnit(const unit::Tower& _unit, ProtoGame::Tower& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());

		auto& derivedState = std::get<unit::TowerState>(_unit.getUnitState().derivedState);
		_msg.set_attackprogress(setHighestBit(static_cast<std::uint32_t>(derivedState.attackProgressPercent)));
		std::uint32_t serializedCD = 0;
		// we serialize the windup cd separate, because we want to make the time until jump visible
		if (_unit.getCooldownTimer().getWindupCooldown().isActive())
		{
			auto& windupTimer = _unit.getCooldownTimer().getWindupCooldown();
			serializedCD = static_cast<std::uint32_t>(windupTimer.getTimerValueInMsec().count() / 100) |
				static_cast<std::uint32_t>(windupTimer.getProgressInPercent()) << 16;
		}
		else
		{
			serializedCD = static_cast<std::uint32_t>(unit::getRemainingTime(_unit.getCooldownTimer()).count() / 100) |
				static_cast<std::uint32_t>(unit::getProgressInPercent(_unit.getCooldownTimer())) << 16;
		}
		_msg.set_cooldown(setHighestBit(serializedCD));
		_msg.set_directionvector(setHighestBit(_serializeDirectionVector(derivedState.directionVector)));
		_msg.set_attackradius(setHighestBit(static_cast<std::uint32_t>(_unit.getUnitState().stats.getStat(unit::StatType::attackRange) * 1000)));
	}

	void _serializeUnit(const unit::Trigger& _unit, ProtoGame::Trigger& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());
	}

	void _serializeUnit(const unit::Projectile& _unit, ProtoGame::Projectile& _msg)
	{
		_serializeUnit(_unit, *_msg.mutable_unit());

		auto& derivedState = std::get<unit::ProjectileState>(_unit.getUnitState().derivedState);
		_msg.set_directionvector(setHighestBit(_serializeDirectionVector(derivedState.directionVector)));
		_msg.set_startposition(setHighestBit(_serializePosition(_unit.getStartPosition())));
		_msg.set_destinationposition(setHighestBit(_unit.getDestination() ? _serializePosition(*_unit.getDestination()) : 0 ));
	}

	template <class UnitsMsg, class UnitsOut>
	void _serializeUnits(UnitsMsg& _msg, const UnitsOut& _out, const Participant& _participant, const map::VisibilityMap* _visMap)
	{
		for (auto unit : _visMap ? _selectVisibleUnits(_out, *_visMap) : _out)
			_serializeUnit(*unit, *_msg.Add());
	}

	using Factories = google::protobuf::RepeatedPtrField<ProtoGame::Factory>;
	void _serializeFactories(Factories& _msg, const device::Factories& _factories)
	{
		for (auto& factory : _factories)
			factory->serialize(*_msg.Add());
	}

	using Abilities = google::protobuf::RepeatedPtrField<ProtoGame::Ability>;
	void _serializeAbilities(Abilities& _msg, const device::Abilities& _abilities)
	{
		for (auto& ability : _abilities)
			ability->serialize(*_msg.Add());
	}

	void _serializeVisibilityMap(ProtoGame::Player& _player, const map::VisibilityMap& _visMap)
	{
		auto msgPtr = std::make_unique<std::string>();
		auto& msg = *msgPtr;
		auto& mapSize = _visMap.getSize();
		msg.reserve(mapSize.getX() * mapSize.getY() / 4);
		std::size_t i = 0;
		for (int y = 0; y < mapSize.getY(); ++y)
		{
			for (int x = 0; x < mapSize.getX(); ++x)
			{
				if (i % 4 == 0)
					msg.push_back(static_cast<std::uint8_t>(_visMap.getTileState({ x, y })));
				else
					msg.back() |= static_cast<std::uint8_t>(_visMap.getTileState({ x, y })) << (i % 4 * 2);	// 2 bits for each cell
				++i;
			}
		}
		_player.set_allocated_visibilitymap(msgPtr.release());
	}

	void _serializeBuildingMap(ProtoGame::Player& _player, const map::BuildingMap& _buildingMap)
	{
		auto msgPtr = std::make_unique<std::string>();
		auto& msg = *msgPtr;
		auto& mapSize = _buildingMap.getBuildingMap().getSize();
		msg.reserve(mapSize.getX() * mapSize.getY() / 8);
		assert(msg.empty());
		std::size_t i = 0;
		for (auto value : _buildingMap.getBuildingMap())
		{
			if (i % 8 == 0)
				msg.push_back(value ? 1 : 0);
			else
				msg.back() |= (value ? 1 : 0) << (i % 8);			// 1 bit for each cell
			++i;
		}
		_player.set_allocated_buildingmap(msgPtr.release());
	}

	void _serializePlayer(ProtoGame::Player& _msg, const phase::AbstractGame& _phase, const Participant& _participant, const std::optional<int>& _currentParticipant)
	{
		if (auto& emote = _phase.getVisualCollector().getEmote(_participant.getIndex()))
			_msg.set_emoteid(*emote);
		_msg.set_minionlimit(setHighestBit(std::min<std::uint32_t>(getMinionLimitInPercent(_participant), 100)));
		_msg.set_colorid(setHighestBit(_participant.getColorID()));
		_msg.set_teamid(_participant.getTeam().getId());
		if (!_participant.getSkins().empty())
			*_msg.mutable_skins() = _serializeOptionalIds(_participant.getSkins());

		_serializeUnit(_participant.getCore(), *_msg.mutable_core());

		auto visMap = _currentParticipant ? &_phase.getParticipants()[*_currentParticipant]->getVisibilityMap() : nullptr;
		_serializeUnits(*_msg.mutable_minions(), _participant.getMinions(), _participant, visMap);
		_serializeUnits(*_msg.mutable_walls(), _participant.getWalls(), _participant, visMap);
		_serializeUnits(*_msg.mutable_towers(), _participant.getTowers(), _participant, visMap);
		_serializeUnits(*_msg.mutable_projectiles(), _participant.getProjectiles(), _participant, visMap);
		_serializeUnits(*_msg.mutable_triggers(), _participant.getTriggers(), _participant, visMap);
		auto compareGuid = [](const auto& _lhs, const auto& _rhs) { return _lhs.unit().guid() < _rhs.unit().guid(); };
		assert(std::is_sorted(std::begin(_msg.minions()), std::end(_msg.minions()), compareGuid) &&
			std::is_sorted(std::begin(_msg.walls()), std::end(_msg.walls()), compareGuid) &&
			std::is_sorted(std::begin(_msg.towers()), std::end(_msg.towers()), compareGuid) &&
			std::is_sorted(std::begin(_msg.projectiles()), std::end(_msg.projectiles()), compareGuid) &&
			std::is_sorted(std::begin(_msg.triggers()), std::end(_msg.triggers()), compareGuid));

		for (auto& visual : _phase.getVisualCollector().getVisuals(_participant.getIndex()))
		{
			auto& visualMsg = *_msg.add_visuals();
			visualMsg.set_position(_serializePosition(visual.getPosition()));
			visualMsg.set_id(visual.getId());

			if (visual.getArea().hasData<data::Shape::Circle>())
				visualMsg.set_circleradius(*visual.getArea().getData<data::Shape::Circle>().radius * 1000);
			else if (visual.getArea().hasData<data::Shape::Rect>())
			{
				auto& rect = visual.getArea().getData<data::Shape::Rect>();
				visualMsg.set_rectsize(static_cast<std::uint32_t>(*rect.width * 1000) |
					(static_cast<std::uint32_t>(*rect.height * 1000) << 16));
			}
		}

		if (!_currentParticipant || *_currentParticipant == _participant.getIndex())
		{
			_serializeFactories(*_msg.mutable_factories(), _participant.getFactories());
			_serializeAbilities(*_msg.mutable_abilities(), _participant.getAbilities());
			_serializeVisibilityMap(_msg, _participant.getVisibilityMap());
			_serializeBuildingMap(_msg, _participant.getBuildingMap());

			if (auto globalTarget = _participant.getGlobalTarget())
			{
				auto& globalTargetMsg = *_msg.mutable_globaltarget();
				globalTargetMsg.set_isset(true);
				globalTargetMsg.set_guid(globalTarget->getGUID());
			}

			_msg.set_flags(setHighestBit(*_participant.getProgressFlags()));
			if (auto minion = findForemostMinion(_participant))
			{
				auto& foremostMinionMsg = *_msg.mutable_foremostminion();
				foremostMinionMsg.set_isset(true);
				foremostMinionMsg.set_guid(minion->getGUID());
			}
		}
	}

	using Players = google::protobuf::RepeatedPtrField<ProtoGame::Player>;
	void _serializePlayers(Players& _msg, const phase::AbstractGame& _phase, const std::optional<int>& _currentParticipant)
	{
		for (auto& participant : _phase.getParticipants())
		{
			auto& player = *_msg.Add();
			_serializePlayer(player, _phase, *participant, _currentParticipant);
		}
	}
} // namespace game::phase_game

namespace game::phase_finalize
{
	using ProtoFinalize = protobuf::out::Match::Finalize;
	void _serializePlayer(ProtoFinalize::Player& _msg, const phase::Finalize& _phase, const FinalizeParticipant& _participant, const std::optional<int>& _currentParticipant)
	{
		assert(_participant.getDefinition().data);
		auto& participantData = *_participant.getDefinition().data;
		
		_msg.set_colorid(setHighestBit(participantData.getColorId()));
		_msg.set_icon(setHighestBit(participantData.getIconId()));
		_msg.set_teamid(_participant.getTeam().getId());
		auto& nameMsg = *_msg.mutable_name();
		nameMsg.set_isset(true);
		nameMsg.set_text(participantData.getName().data());

		if (auto& emote = _phase.getVisualCollector().getEmote(_participant.getIndex()))
			_msg.set_emoteid(*emote);
	}

	using Players = google::protobuf::RepeatedPtrField<ProtoFinalize::Player>;
	void _serializePlayers(Players& _msg, const phase::Finalize& _phase, const std::optional<int>& _currentParticipant)
	{
		for (auto& participant : _phase.getParticipants())
		{
			auto& player = *_msg.Add();
			_serializePlayer(player, _phase, *participant, _currentParticipant);
		}
	}
} // game::phase_finalize

namespace game
{
	protobuf::out::Message serializeToProtobuf(const phase::Pick& _phase, std::optional<int> _playerIndex)
	{
		protobuf::out::Message msg;
		auto& pickMsg = *msg.mutable_match()->mutable_pick();

		using namespace phase_pick;
		_serializePickPlayers(*pickMsg.mutable_player(), _phase, _playerIndex);

		pickMsg.set_activeplayerindex(setHighestBit(_phase.getActiveParticipantIndex()));
		pickMsg.set_playerindex(setHighestBit(_playerIndex.value_or(0)));
		pickMsg.set_state(static_cast<ProtoPick::State>(_phase.getState()));
		pickMsg.set_time(static_cast<std::uint32_t>(_phase.getTime().count() / 100));
		return msg;
	}

	protobuf::out::Message serializeToProtobuf(const phase::AbstractGame& _phase, std::optional<int> _playerIndex)
	{
		protobuf::out::Message msg;
		auto& gameMsg = *msg.mutable_match()->mutable_game();

		gameMsg.set_playerindex(setHighestBit(_playerIndex.value_or(0)));
		gameMsg.set_gametime(_phase.getGameTimeElapsed().count());
		auto warumUpTime = std::chrono::duration_cast<std::chrono::milliseconds>(_phase.getWarmUpTimer().getTimerValue());
		gameMsg.set_warmuptime(static_cast<std::uint32_t>(warumUpTime .count() / 100));

		using namespace phase_game;
		_serializePlayers(*gameMsg.mutable_player(), _phase, _playerIndex);
		return msg;
	}

	protobuf::out::Message serializeToProtobuf(const phase::Finalize& _phase, std::optional<int> _playerIndex)
	{
		protobuf::out::Message msg;
		auto& finalizeMsg = *msg.mutable_match()->mutable_finalize();

		finalizeMsg.set_playerindex(setHighestBit(_playerIndex.value_or(0)));
		finalizeMsg.set_permmatchid(_phase.getPermanentMatchId().value_or(0));
		finalizeMsg.set_timeleft(static_cast<std::uint32_t>(_phase.getPhaseTimer().getTimerValue().count() / 100));
		finalizeMsg.set_winningteamid(setHighestBit(_phase.getWinningTeamId().value_or(0)));

		using namespace phase_finalize;
		_serializePlayers(*finalizeMsg.mutable_players(), _phase, _playerIndex);
		return msg;
	}
} // namespace game
