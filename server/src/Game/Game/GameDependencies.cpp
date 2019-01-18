#include "stdafx.h"
#include "GameDependencies.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"

namespace game {

	GameDependencies::GameDependencies(phase::AbstractGame& _gamePhase, unit::World& _world) :
		tileMap(_gamePhase.getTileMap()),
		pathMap(_gamePhase.getPathMap()),
		colliderMap(_gamePhase.getColliderMap()),
		world(_world),
		gamePhase(_gamePhase),
		gameData(_gamePhase.getGameData()),
		spellManager(_gamePhase.getSpellManager())
	{
	}
} // namespace game
