#include "stdafx.h"
#include "BaseAI.hpp"
#include "Core/ParticipantData.hpp"
#include "Data/Game.hpp"

namespace game::ai
{
	BaseAI::BaseAI(const Initializer& _initializer) :
		m_GameData(_initializer.gameData),
		m_BotLevel(_initializer.botLevel)
	{
	}

	const data::Game& BaseAI::getGameData() const
	{
		return m_GameData;
	}

	int BaseAI::getBotLevel() const
	{
		assert(0 <= m_BotLevel);
		return m_BotLevel;
	}

	ParticipantData generateParticipantData(const data::Game& _gameData, int _botLevel, const data::AiStrategy* _strategy)
	{
		if (_gameData.colors.empty())
			throw std::runtime_error("AI: unable to select random color; color table is empty.");
		auto colorItr = std::begin(_gameData.colors);
		std::advance(colorItr, randomInt<std::size_t>(0, _gameData.colors.size() - 1));
		return ParticipantData(ParticipantData::Bot{ _botLevel }, "Bot (Level " + std::to_string(_botLevel) + ")", 0, _strategy ? _strategy->skins : IDs(),
			createAllUnlocks(_gameData), *colorItr->id, _strategy ? _strategy->modPages : std::vector<ModPage>(), {});
	}

	Unlocks createAllUnlocks(const data::Game& _gameData)
	{
		Unlocks result;
		for (auto& unlock : _gameData.unlocks)
			result[static_cast<std::size_t>(unlock.typeId) - 1].emplace_back(*unlock.id);
		for (auto& ids : result)
		{
			std::sort(std::begin(ids), std::end(ids));
			ids.erase(std::unique(std::begin(ids), std::end(ids)), std::end(ids));
		}
		return result;
	}
} // namespace game::ai