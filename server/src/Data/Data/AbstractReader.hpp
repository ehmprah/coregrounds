#pragma once

#include "Core/StdInclude.hpp"
#include "DataFwd.hpp"
#include "Game.hpp"

namespace game::data
{
	class AbstractReader
	{
	protected:
		virtual GameProperties readGameProperties() = 0;
		virtual Table<Spell> readSpells(const GameProperties& _properties) = 0;
		virtual Table<Aura> readAuras(const GameProperties& _properties) = 0;
		virtual Table<Ability> readAbilities(const GameProperties& _properties) = 0;

		virtual Table<Factory> readFactories(const GameProperties& _properties) = 0;

		virtual Table<Core> readCores(const GameProperties& _properties) = 0;
		virtual Table<Wall> readWalls(const GameProperties& _properties) = 0;
		virtual Table<Tower> readTowers(const GameProperties& _properties) = 0;
		virtual Table<Minion> readMinions(const GameProperties& _properties) = 0;
		virtual Table<Projectile> readProjectiles(const GameProperties& _properties) = 0;
		virtual Table<Trigger> readTriggers(const GameProperties& _properties) = 0;

		virtual Table<Color> readColors(const GameProperties& _properties) = 0;
		virtual Table<PlayerIcon> readPlayerIcons(const GameProperties& _properties) = 0;
		virtual Table<Skin> readSkins(const GameProperties& _properties) = 0;
		virtual Table<Background> readBackgrounds(const GameProperties& _properties) = 0;
		virtual Table<Modification> readModifications(const GameProperties& _properties) = 0;
		virtual Table<Emote> readEmotes(const GameProperties& _properties) = 0;

		virtual Table<AiMazeLayout> readAiMazeLayouts(const GameProperties& _properties) = 0;
		virtual Table<AiStrategy> readAiStrategies(const GameProperties& _properties) = 0;

	public:
		virtual ~AbstractReader() = default;

		virtual bool isOpen() const = 0;
		std::shared_ptr<Game> read();
	};
	using ReaderPtr = std::unique_ptr<AbstractReader>;
} // namespace game::data
