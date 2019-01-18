#pragma once

#include "PhaseInterface.hpp"
#include "Game/Entity/Participant.hpp"
#include "Game/map/ColliderMap.hpp"
#include "Game/map/TileMap.hpp"
#include "Game/Map/PathMap.hpp"
#include "Game/Entity/World.hpp"
#include "Game/Spell/SpellManager.hpp"
#include "Game/Entity/Team.hpp"
#include "Data/Game.hpp"

namespace game::phase
{
	class AbstractGame :
		public Interface
	{
		using super = Interface;

	public:
		using Participants = std::array<ParticipantPtr, 2>;

		AbstractGame(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _gameData);
		virtual ~AbstractGame() = default;

		Time getGameTimeElapsed() const;
		const Timer& getWarmUpTimer() const;

		std::optional<int> getWinningTeamId() const;

		spell::SpellManager& getSpellManager();
		const data::Game& getGameData() const;
		map::ColliderMap& getColliderMap();
		map::TileMap& getTileMap();
		map::PathMap& getPathMap();
		unit::World& getWorld();
		const unit::World& getWorld() const;

		const Participants& getParticipants() const;
		void setup(std::vector<ParticipantDef> _definitions);

		virtual void start() override;
		virtual void finalize() override;

		virtual Data update(Time _diff) override;

		virtual bool hasFinished() const override;
		bool isValid() const;
		virtual Type getType() const override;

		VisualCollector& getVisualCollector() override;
		const VisualCollector& getVisualCollector() const override;

	private:
		std::shared_ptr<const data::Game> m_GameData;
		const data::GameProperties::Game& m_GameProperties;
		Participants m_Participants;
		const std::vector<Team>& m_Teams;

		map::ColliderMap m_ColliderMap;
		map::TileMap m_TileMap;
		map::PathMap m_PathMap;
		unit::World m_World;
		spell::SpellManager m_SpellManager;
		VisualCollector m_VisualCollector;

		GameDependencies m_GameDependencies;

		std::chrono::time_point<std::chrono::steady_clock, Time> m_GameBegin;
		std::chrono::time_point<std::chrono::steady_clock, Time> m_GameEnded;

		Timer m_WarmUpTimer;

		bool m_Finished = false;
	};
	using GamePtr = std::unique_ptr<AbstractGame>;

	bool isRunning(const AbstractGame& _phase);
} // namespace game::phase
