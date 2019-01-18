#include "AbstractReader.hpp"

namespace game::data
{
	template <class TData>
	UnlockType _selectUnlockType()
	{
		if constexpr (std::is_same_v<Ability, TData>)
			return UnlockType::ability;
		else if constexpr (std::is_same_v<Background, TData>)
			return UnlockType::background;
		else if constexpr (std::is_same_v<Color, TData>)
			return UnlockType::color;
		else if constexpr (std::is_same_v<Emote, TData>)
			return UnlockType::emote;
		else if constexpr (std::is_same_v<Factory, TData>)
			return UnlockType::factory;
		else if constexpr (std::is_same_v<Modification, TData>)
			return UnlockType::modification;
		else if constexpr (std::is_same_v<PlayerIcon, TData>)
			return UnlockType::playerIcon;
		else if constexpr (std::is_same_v<Skin, TData>)
			return UnlockType::skin;
		else
			static_assert(false, "Invalid type");
	}

	template <class TData>
	UnlockType _selectUnlockType(const TData&)
	{
		return _selectUnlockType<TData>();
	}

	template <class TStream>
	TStream& operator <<(TStream& _out, UnlockType _type)
	{
		switch (_type)
		{
		case UnlockType::ability: _out << "ability"; break;
		case UnlockType::background: _out << "background"; break;
		case UnlockType::color: _out << "color"; break;
		case UnlockType::emote: _out << "emote"; break;
		case UnlockType::factory: _out << "factory"; break;
		case UnlockType::modification: _out << "modification"; break;
		case UnlockType::playerIcon: _out << "playerIcon"; break;
		case UnlockType::skin: _out << "skin"; break;
		}
		return _out;
	}

	template <class TTable>
	void _extractUnlocks(Table<Unlock>& _unlocks, const TTable& _table)
	{
		using ValueType = typename TTable::value_type;
		LOG_CHANNEL_INFO(dataLogger, "Begin extracting unlocks: " << _selectUnlockType<ValueType>());
		for (auto& el : _table)
		{
			Unlock unlock{ el.id, _selectUnlockType(el) };
			if (!data::insertData(_unlocks, unlock))
				LOG_CHANNEL_ERR(dataLogger, "Found unlock duplication id: " << *unlock.id << " type: " << unlock.typeId);
		}
		LOG_CHANNEL_INFO(dataLogger, "Finished extracting unlocks: " << _selectUnlockType<ValueType>());
	}

	std::shared_ptr<Game> AbstractReader::read()
	{
		if (!isOpen())
			throw Error("GameDataReader: File not open.");

		auto gameData = std::make_shared<Game>();
		gameData->gameProperties = readGameProperties();
		gameData->gameProperties.checkAndAdjust();

		// spells and ability
		gameData->spells = readSpells(gameData->gameProperties);
		gameData->auras = readAuras(gameData->gameProperties);
		gameData->abilities = readAbilities(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->abilities);

		gameData->factories = readFactories(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->factories);

		// units
		gameData->cores = readCores(gameData->gameProperties);
		gameData->walls = readWalls(gameData->gameProperties);
		gameData->towers = readTowers(gameData->gameProperties);
		gameData->minions = readMinions(gameData->gameProperties);
		gameData->projectiles = readProjectiles(gameData->gameProperties);
		gameData->triggers = readTriggers(gameData->gameProperties);

		// others
		gameData->colors = readColors(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->colors);
		gameData->playerIcons = readPlayerIcons(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->playerIcons);
		gameData->skins = readSkins(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->skins);
		gameData->backgrounds = readBackgrounds(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->backgrounds);
		gameData->modifications = readModifications(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->modifications);
		gameData->emotes = readEmotes(gameData->gameProperties);
		_extractUnlocks(gameData->unlocks, gameData->emotes);

		// ai
		gameData->aiMazeLayouts = readAiMazeLayouts(gameData->gameProperties);
		gameData->aiStrategies = readAiStrategies(gameData->gameProperties);
		return gameData;
	}
} // namespace game::data
