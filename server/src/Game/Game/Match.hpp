#pragma once

#include "Game/Phase/PickPhase.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"
#include "Game/Phase/FinalizePhase.hpp"
#include "Game/controller/ControllerBase.hpp"

namespace game
{
	class Statistics;

	using GameData = std::shared_ptr<const data::Game>;

namespace controller
{
	class Handler;
} // namespace controller

	class MatchDef
	{
	public:
		int tempId;

		std::string readableMatchType;

		enum class PickMode
		{
			normal,
			random
		} pickMode;

		enum class GameMode
		{
			normal
		} gameMode;

		using ParticipantDef = std::variant<int/*botLevel*/, std::weak_ptr<game::controller::Handler>>;
		std::vector<ParticipantDef> participants;
	};

	class Match :
		sl2::NonCopyable
	{
	public:
		enum class StateType
		{
			notStarted,
			pick,
			game,
			finalize,
			finished
		};

		Match(MatchDef _matchDef, const GameData& _gameData);

		struct State
		{
			StateType type;
			phase::Data data;
		};
		State update(Time _diff);
		bool isValid() const;

	private:
		MatchDef m_MatchDef;
		std::vector<Team> m_Teams;

		StateType m_State = StateType::notStarted;

		phase::Interface* m_CurrentPhase = nullptr;
		phase::PickPtr m_PickPhase;
		phase::GamePtr m_GamePhase;
		phase::FinalizePtr m_FinalizePhase;

		controller::InputHandler m_InputHandler;
		std::vector<controller::ControllerPtr> m_Controllers;

		void _switchNextPhase();
		Statistics _generateStatistics() const;
	};
	using MatchPtr = std::unique_ptr<Match>;

	MatchPtr createMatch(MatchDef _matchDef, const GameData& _gameData);
} // namespace game
