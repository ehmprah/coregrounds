#include "stdafx.h"
#include "Patching.hpp"

using namespace protobuf::out;

#define PROTO_PATCH(old, msg, patch, fieldName) if (old.fieldName() != msg.fieldName()) patch.set_##fieldName(msg.fieldName())
#define PROTO_PATCH_MUTABLE(old, msg, patch, fieldName) if (old.fieldName() != msg.fieldName()) *patch.mutable_##fieldName() = msg.fieldName()

template <class T>
bool operator ==(const google::protobuf::RepeatedField<T>& _lhs, const google::protobuf::RepeatedField<T>& _rhs)
{
	return std::equal(std::begin(_lhs), std::end(_lhs), std::begin(_rhs), std::end(_rhs));
}

template <class T>
bool operator !=(const google::protobuf::RepeatedField<T>& _lhs, const google::protobuf::RepeatedField<T>& _rhs)
{
	return !(_lhs == _rhs);
}

bool operator ==(const Match::Game::OptionalGUID& _lhs, const Match::Game::OptionalGUID& _rhs)
{
	return _lhs.isset() == _rhs.isset() && (!_lhs.isset() || _lhs.guid() == _rhs.guid());
}

bool operator !=(const Match::Game::OptionalGUID& _lhs, const Match::Game::OptionalGUID& _rhs)
{
	return !(_lhs == _rhs);
}

bool operator ==(const Match::OptionalIds& _lhs, const Match::OptionalIds& _rhs)
{
	return _lhs.isset() == _rhs.isset() &&
		std::equal(std::begin(_lhs.ids()), std::end(_lhs.ids()), std::begin(_rhs.ids()), std::end(_rhs.ids()));
}

bool operator !=(const Match::OptionalIds& _lhs, const Match::OptionalIds& _rhs)
{
	return !(_lhs == _rhs);
}

bool operator ==(const Match::OptionalString& _lhs, const Match::OptionalString& _rhs)
{
	return _lhs.isset() == _rhs.isset() && _lhs.text() == _rhs.text();
}

bool operator !=(const Match::OptionalString& _lhs, const Match::OptionalString& _rhs)
{
	return !(_lhs == _rhs);
}

namespace game::patch_game
{
	using Unit = Match::Game::Unit;
	void _patchUnit(const Unit& _old, const Unit& _new, Unit& _patch)
	{
		assert(_old.guid() == _new.guid());
		_patch.set_guid(_new.guid());
		*_patch.mutable_stats() = _new.stats();

		PROTO_PATCH(_old, _new, _patch, id);
		PROTO_PATCH(_old, _new, _patch, hp);
		PROTO_PATCH(_old, _new, _patch, position);
		PROTO_PATCH(_old, _new, _patch, state);
		PROTO_PATCH_MUTABLE(_old, _new, _patch, auravisualids);
	}

	using Core = Match::Game::Core;
	void _patchUnit(const Core& _old, const Core& _new, Core& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());
	}

	using Minion = Match::Game::Minion;
	void _patchUnit(const Minion& _old, const Minion& _new, Minion& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());

		PROTO_PATCH(_old, _new, _patch, attackprogress);
		PROTO_PATCH(_old, _new, _patch, directionvector);
	}

	using Wall = Match::Game::Wall;
	void _patchUnit(const Wall& _old, const Wall& _new, Wall& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());

		PROTO_PATCH(_old, _new, _patch, chainbonus);
	}

	using Tower = Match::Game::Tower;
	void _patchUnit(const Tower& _old, const Tower& _new, Tower& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());

		PROTO_PATCH(_old, _new, _patch, attackprogress);
		PROTO_PATCH(_old, _new, _patch, cooldown);
		PROTO_PATCH(_old, _new, _patch, directionvector);
		PROTO_PATCH(_old, _new, _patch, attackradius);
	}

	using Projectile = Match::Game::Projectile;
	void _patchUnit(const Projectile& _old, const Projectile& _new, Projectile& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());

		PROTO_PATCH(_old, _new, _patch, startposition);
		PROTO_PATCH(_old, _new, _patch, destinationposition);
		PROTO_PATCH(_old, _new, _patch, directionvector);
	}

	using Trigger = Match::Game::Trigger;
	void _patchUnit(const Trigger& _old, const Trigger& _new, Trigger& _patch)
	{
		_patchUnit(_old.unit(), _new.unit(), *_patch.mutable_unit());
	}

	template <class Units>
	void _patchUnits(const Units& _old, const Units& _new, Units& _patch)
	{
		for (auto& newUnit : _new)
		{
			auto compareGUID = [guid = newUnit.unit().guid()](const auto& unit) { return unit.unit().guid() == guid; };
			if (auto itr = std::find_if(std::begin(_old), std::end(_old), compareGUID); itr != std::end(_old))
				_patchUnit(*itr, newUnit, *_patch.Add());
			else
				*_patch.Add() = newUnit;
		}
	}

	using WallFactory = Match::Game::Factory::Wall;
	void _patchWallFactory(const WallFactory& _old, const WallFactory& _new, WallFactory& _patch)
	{
		PROTO_PATCH(_old, _new, _patch, stacks);
	}

	using TowerFactory = Match::Game::Factory::Tower;
	void _patchTowerFactory(const TowerFactory& _old, const TowerFactory& _new, TowerFactory& _patch)
	{
		_patch.set_guid(_new.guid());
		PROTO_PATCH(_old, _new, _patch, level);
		PROTO_PATCH(_old, _new, _patch, cooldown);
		PROTO_PATCH(_old, _new, _patch, upgrades);
		PROTO_PATCH(_old, _new, _patch, targetpreference);
	}

	using MinionFactory = Match::Game::Factory::Minion;
	void _patchMinionFactory(const MinionFactory& _old, const MinionFactory& _new, MinionFactory& _patch)
	{
		PROTO_PATCH(_old, _new, _patch, level);
		PROTO_PATCH(_old, _new, _patch, stacks);
		PROTO_PATCH(_old, _new, _patch, cooldown);
		PROTO_PATCH(_old, _new, _patch, upgrades);
		PROTO_PATCH(_old, _new, _patch, targetpreference);
	}

	using Factories = google::protobuf::RepeatedPtrField<Match::Game::Factory>;
	void _patchFactories(const Factories& _old, const Factories& _new, Factories& _patch)
	{
		assert(std::size(_old) == std::size(_new));
		for (auto oldItr = std::begin(_old), newItr = std::begin(_new);
			oldItr != std::end(_old);
			++oldItr, ++newItr)
		{
			auto& oldFc = *oldItr;
			auto& newFc = *newItr;
			auto& patchFc = *_patch.Add();

			*patchFc.mutable_stats() = newFc.stats();

			PROTO_PATCH(oldFc, newFc, patchFc, id);
			PROTO_PATCH_MUTABLE(oldFc, newFc, patchFc, auravisualids);
			if (oldFc.has_wall())
				_patchWallFactory(oldFc.wall(), newFc.wall(), *patchFc.mutable_wall());
			else if (oldFc.has_tower())
				_patchTowerFactory(oldFc.tower(), newFc.tower(), *patchFc.mutable_tower());
			else if (oldFc.has_minion())
				_patchMinionFactory(oldFc.minion(), newFc.minion(), *patchFc.mutable_minion());
		}
	}

	using Abilities = google::protobuf::RepeatedPtrField<Match::Game::Ability>;
	void _patchAbilities(const Abilities& _old, const Abilities& _new, Abilities& _patch)
	{
		assert(std::size(_old) == std::size(_new));
		for (auto oldItr = std::begin(_old), newItr = std::begin(_new);
			oldItr != std::end(_old);
			++oldItr, ++newItr)
		{
			auto& oldAb = *oldItr;
			auto& newAb = *newItr;
			auto& patchAb = *_patch.Add();
			*patchAb.mutable_stats() = newAb.stats();

			PROTO_PATCH(oldAb, newAb, patchAb, id);
			PROTO_PATCH(oldAb, newAb, patchAb, cooldown);
			PROTO_PATCH_MUTABLE(oldAb, newAb, patchAb, auravisualids);
		}
	}

	using Players = google::protobuf::RepeatedPtrField<Match::Game::Player>;
	void _patchPlayers(const Players& _old, const Players& _new, Players& _patch)
	{
		assert(std::size(_old) == std::size(_new));
		for (auto oldItr = std::begin(_old), newItr = std::begin(_new);
			oldItr != std::end(_old);
			++oldItr, ++newItr)
		{
			auto& oldPl = *oldItr;
			auto& newPl = *newItr;
			auto& patchPl = *_patch.Add();

			patchPl.set_emoteid(newPl.emoteid());
			*patchPl.mutable_visuals() = newPl.visuals();
			_patchUnit(oldPl.core(), newPl.core(), *patchPl.mutable_core());
			_patchUnits(oldPl.minions(), newPl.minions(), *patchPl.mutable_minions());
			_patchUnits(oldPl.walls(), newPl.walls(), *patchPl.mutable_walls());
			_patchUnits(oldPl.towers(), newPl.towers(), *patchPl.mutable_towers());
			_patchUnits(oldPl.projectiles(), newPl.projectiles(), *patchPl.mutable_projectiles());
			_patchUnits(oldPl.triggers(), newPl.triggers(), *patchPl.mutable_triggers());
			_patchAbilities(oldPl.abilities(), newPl.abilities(), *patchPl.mutable_abilities());
			_patchFactories(oldPl.factories(), newPl.factories(), *patchPl.mutable_factories());
			PROTO_PATCH(oldPl, newPl, patchPl, minionlimit);
			PROTO_PATCH(oldPl, newPl, patchPl, colorid);
			PROTO_PATCH(oldPl, newPl, patchPl, teamid);
			PROTO_PATCH(oldPl, newPl, patchPl, visibilitymap);
			PROTO_PATCH(oldPl, newPl, patchPl, buildingmap);
			PROTO_PATCH(oldPl, newPl, patchPl, flags);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, globaltarget);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, foremostminion);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, skins);
		}
	}
} // namespace game::patch_game

namespace game::patch_pick
{
	using Players = google::protobuf::RepeatedPtrField<Match::Pick::Player>;
	void _patchPlayers(const Players& _old, const Players& _new, Players& _patch)
	{
		assert(std::size(_old) == std::size(_new));
		for (auto oldItr = std::begin(_old), newItr = std::begin(_new);
			oldItr != std::end(_old);
			++oldItr, ++newItr)
		{
			auto& oldPl = *oldItr;
			auto& newPl = *newItr;
			auto& patchPl = *_patch.Add();

			patchPl.set_emoteid(newPl.emoteid());
			PROTO_PATCH(oldPl, newPl, patchPl, modpageid);
			PROTO_PATCH(oldPl, newPl, patchPl, colorid);
			PROTO_PATCH(oldPl, newPl, patchPl, teamid);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, availableids);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, factorybans);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, abilitybans);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, factorypicks);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, abilitypicks);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, skins);
		}
	}
} // namespace game::patch_pick

namespace game::patch_finalize
{
	using Players = google::protobuf::RepeatedPtrField<Match::Finalize::Player>;
	void _patchPlayers(const Players& _old, const Players& _new, Players& _patch)
	{
		assert(std::size(_old) == std::size(_new));
		for (auto oldItr = std::begin(_old), newItr = std::begin(_new);
			oldItr != std::end(_old);
			++oldItr, ++newItr)
		{
			auto& oldPl = *oldItr;
			auto& newPl = *newItr;
			auto& patchPl = *_patch.Add();

			patchPl.set_emoteid(newPl.emoteid());
			PROTO_PATCH(oldPl, newPl, patchPl, colorid);
			PROTO_PATCH(oldPl, newPl, patchPl, teamid);
			PROTO_PATCH(oldPl, newPl, patchPl, icon);
			PROTO_PATCH_MUTABLE(oldPl, newPl, patchPl, name);
		}
	}
} // namespace game::patch_finalize

namespace game
{
	std::unique_ptr<Match::Game> _patchGame(const Match::Game& _old, const Match::Game& _new)
	{
		auto patch = std::make_unique<Match::Game>();
		auto& patchRef = *patch;

		patchRef.set_warmuptime(_new.warmuptime());
		patchRef.set_gametime(_new.gametime());

		PROTO_PATCH(_old, _new, patchRef, playerindex);

		using namespace patch_game;
		_patchPlayers(_old.player(), _new.player(), *patchRef.mutable_player());
		return patch;
	}

	std::unique_ptr<Match::Pick> _patchPick(const Match::Pick& _old, const Match::Pick& _new)
	{
		auto patch = std::make_unique<Match::Pick>();
		auto& patchRef = *patch;
		patchRef.set_state(_new.state());
		patchRef.set_time(_new.time());

		PROTO_PATCH(_old, _new, patchRef, playerindex);
		PROTO_PATCH(_old, _new, patchRef, activeplayerindex);

		using namespace patch_pick;
		_patchPlayers(_old.player(), _new.player(), *patchRef.mutable_player());
		return patch;
	}

	std::unique_ptr<Match::Finalize> _patchFinalize(const Match::Finalize& _old, const Match::Finalize& _new)
	{
		auto patch = std::make_unique<Match::Finalize>();
		auto& patchRef = *patch;

		PROTO_PATCH(_old, _new, patchRef, playerindex);
		PROTO_PATCH(_old, _new, patchRef, permmatchid);
		PROTO_PATCH(_old, _new, patchRef, timeleft);

		using namespace patch_finalize;
		_patchPlayers(_old.players(), _new.players(), *patchRef.mutable_players());
		return patch;
	}

	std::unique_ptr<Match> _patchMatch(const Match& _old, const Match& _new)
	{
		auto patch = std::make_unique<Match>();
		if (_old.Phase_case() != _new.Phase_case())
			*patch = _new;
		else if (_old.has_game())
			patch->set_allocated_game(_patchGame(_old.game(), _new.game()).release());
		else if (_old.has_pick())
			patch->set_allocated_pick(_patchPick(_old.pick(), _new.pick()).release());
		else if (_old.has_finalize())
			patch->set_allocated_finalize(_patchFinalize(_old.finalize(), _new.finalize()).release());
		return patch;
	}

	Message createPatch(const Message& _old, const Message& _new)
	{
		if (_old.event_case() != _new.event_case())
			return _new;

		Message msg;
		if (_old.has_match())
		{
			msg.set_allocated_match(_patchMatch(_old.match(), _new.match()).release());
		}
		return msg;
	}
} // namespace game
