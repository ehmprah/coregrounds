#pragma once

#include "Data/AbstractReader.hpp"

namespace game {
namespace data {

	ReaderPtr createReaderForFile(std::string_view _pathName);

	class JsonReader : public AbstractReader
	{
	private:
		const json::Object& m_Doc;

		const json::Object& _getRootObject();

		GameProperties readGameProperties() override;
		Table<Spell> readSpells(const GameProperties& _properties) override;
		Table<Aura> readAuras(const GameProperties& _properties) override;

		Table<Ability> readAbilities(const GameProperties& _properties) override;
		Table<Factory> readFactories(const GameProperties& _properties) override;

		Table<Core> readCores(const GameProperties& _properties) override;
		Table<Wall> readWalls(const GameProperties& _properties) override;
		Table<Tower> readTowers(const GameProperties& _properties) override;
		Table<Minion> readMinions(const GameProperties& _properties) override;
		Table<Projectile> readProjectiles(const GameProperties& _properties) override;
		Table<Trigger> readTriggers(const GameProperties& _properties) override;

		Table<Color> readColors(const GameProperties& _gameProp) override;
		Table<PlayerIcon> readPlayerIcons(const GameProperties& _gameProp) override;
		Table<Skin> readSkins(const GameProperties& _gameProp) override;
		Table<Background> readBackgrounds(const GameProperties& _gameProp) override;
		Table<Modification> readModifications(const GameProperties& _gameProp) override;
		Table<Emote> readEmotes(const GameProperties& _gameProp) override;

		Table<AiMazeLayout> readAiMazeLayouts(const GameProperties& _properties) override;
		Table<AiStrategy> readAiStrategies(const GameProperties& _properties) override;

	public:
		JsonReader(const json::Object& _doc);

		bool isOpen() const override;
	};
} // namespace data
} // namespace game
