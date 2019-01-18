#pragma once

#include "Config.hpp"
#include "Data/Game.hpp"
#include "session/Session.hpp"
#include "Game/Match.hpp"
#include "Core/ParticipantData.hpp"
#include "Game/GameStatistics.hpp"
#include "SynchronizedValue.hpp"

namespace match
{
	using Id = int;
	using MatchType = std::string;

	using SendMatchStatistics = boost::signals2::signal<void(const game::Statistics&)>;
	using MatchCanceledSignal = boost::signals2::signal<void(Id)>;
	using MatchStartedSignal = boost::signals2::signal<void(Id)>;

	using GameDataPtr = std::shared_ptr<const game::data::Game>;

	using PlayerDef = std::variant<int/*botLevel*/, json::Object/*playerData*/>;

	enum class DefinitionIndex
	{
		tmpId,
		token,
		type,
		playerDefinitions
	};
	using Definition = std::tuple<int, std::string, std::string, std::vector<PlayerDef>>;

	template <DefinitionIndex _index>
	decltype(auto) get(Definition& _def)
	{
		return std::get<static_cast<std::size_t>(_index)>(_def);
	}

	template <DefinitionIndex _index>
	decltype(auto) get(const Definition& _def)
	{
		return std::get<static_cast<std::size_t>(_index)>(_def);
	}

	struct PreparationState
	{
		std::vector<const json::Object*> players;
		std::vector<int> bots;
	};

	class Worker :
		boost::noncopyable
	{
	public:
		Worker(const Definition& _definition, game::MatchPtr _match);

		void operator()();

		void setPermaId(Id _id);
		bool isActive() const;
		void stop();

		SendMatchStatistics onSendMatchStatistics;

	private:
		const Definition& m_Definition;
		game::MatchPtr m_Match;
		std::atomic<std::optional<Id>> m_PermaId;
		std::atomic_bool m_IsActive{ true };
		std::atomic_bool m_ShallStop{ false };

		bool m_HandledMatchStatistics = false;

		bool _handleMatchStates(game::Match::State& _state);
	};

	struct RunningState
	{
		std::unique_ptr<Worker> worker;
		std::thread thread;
	};
	using StateVariables = std::variant<PreparationState, RunningState>;

	class Handler :
		boost::noncopyable
	{
	public:
		Handler(Definition _def, GameDataPtr _gameData);
		~Handler();

		const Definition& getDefinition() const noexcept;
		std::chrono::system_clock::time_point getStartTime() const;

		enum class State
		{
			preparation,
			running,
			finished
		};
		State getState() const noexcept;

		bool addAsParticipant(session::SessionPtr _session);
		void sendMessageToParticipants(protobuf::out::Message _msg);
		// bool addAsSpectator(session::SessionPtr _session);

		void setPermaId(Id _permaId);

		SendMatchStatistics onSendMatchStatistics;

	private:
		std::chrono::system_clock::time_point m_StartTimePoint = std::chrono::system_clock::now();
		Definition m_MatchDef;
		GameDataPtr m_GameData;

		session::SessionPtrs m_Participants;
		std::mutex m_ParticipantsMx;
		session::SessionPtrs m_Spectators;
		std::mutex m_SpectatorsMx;

		mutable std::mutex m_StateVariablesMx;
		StateVariables m_StateVariables;
	};
	bool isActive(const Handler& _match);
	Id getTmpId(const Handler& _match);

	using MatchContainer = std::vector<std::unique_ptr<Handler>>;

	struct MatchInformation
	{
		Definition definition;
		using State = Handler::State;
		State state;
	};

	class Manager
	{
	public:
		Manager(const Config& _config);

		void setPermMatchId(Id _tmpId, Id _permId);
		void setGameData(GameDataPtr _gameData);

		void addMatch(Definition _matchDef);
		void addSessionToMatch(session::SessionPtr _session);

		bool hasMatch(Id _tmpId) const;

		std::vector<MatchInformation> gatherMatchInformations() const;

		void cleanup();

		SendMatchStatistics onSendMatchStatistics;
		MatchCanceledSignal onMatchCanceled;
		MatchStartedSignal onMatchStarted;

	private:
		const Config& m_Config;
		GameDataPtr m_GameData;
		MatchContainer m_Matches;
		mutable std::mutex m_MatchesMx;

		std::optional<Id> _tryAddSessionToMatch(session::SessionPtr _session);
	};
} // namespace match
