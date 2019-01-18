#pragma once

#include <Core/AI.hpp>

namespace game::ai
{
	class BaseAI : public Interface
	{
	public:
		BaseAI(const Initializer& _initializer);

		const data::Game& getGameData() const override;
		int getBotLevel() const override;

	private:
		const data::Game& m_GameData;
		int m_BotLevel = 0;
	};

	Unlocks createAllUnlocks(const data::Game& _gameData);
	ParticipantData generateParticipantData(const data::Game& _gameData, int _botLevel, const data::AiStrategy* _strategy);
} // namespace game::ai