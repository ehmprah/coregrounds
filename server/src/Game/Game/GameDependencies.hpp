#pragma once

namespace game
{
namespace phase
{
	class AbstractGame;
} // namespace phase

	class GameDependencies :
		sl2::NonCopyable
	{
	public:
		const data::Game& gameData;
		unit::World& world;
		map::TileMap& tileMap;
		map::PathMap& pathMap;
		map::ColliderMap& colliderMap;
		spell::SpellManager& spellManager;

		phase::AbstractGame& gamePhase;

		GameDependencies(phase::AbstractGame& _gamePhase, unit::World& _world);
	};
} // namespace game
