#include "stdafx.h"
#include "MatchMgr.hpp"
#include "Host/session/Handler.hpp"
#include "Log.hpp"

namespace match
{
	const json::Object* _findPlayerDefinition(session::TokenView _token, const std::vector<const json::Object*>& _definitions)
	{
		auto itr = std::find_if(std::begin(_definitions), std::end(_definitions),
			[_token](auto _playerDef) { return json::get<std::string>(*_playerDef, "sid") == _token; }
		);
		return itr != std::end(_definitions) ? *itr : nullptr;
	}

	std::vector<game::ModPage> _readModPages(const json::Object& _json)
	{
		std::vector<game::ModPage> modPages;
		if (auto modPageArray = json::getArrayPtr(_json, "pages"))
		{
			for (auto& page : *modPageArray)
			{
				auto pid = json::getValue<game::ID>(page, "pid");
				game::IDs modIds;
				for (auto& mods : json::getArray(page, "mods"))
				{
					assert(mods.is_array());
					modIds.reserve(modIds.size() + mods.size());
					std::transform(std::begin(mods), std::end(mods), std::back_inserter(modIds),
						[](const auto& _id) { return json::getOptValue<game::ID>(_id).value_or(0); }
					);
				}
				modPages.emplace_back(pid, std::move(modIds));
			}
		}
		return modPages;
	}

	game::Unlocks _extractUnlocks(const json::Object& _json, const game::data::Game& _gameData)
	{
		game::Unlocks unlocks;
		for (auto& node : json::getArray(_json, "unlocks"))
		{
			auto id = node.get<game::ID>();
			if (auto unlock = game::data::findData(_gameData.unlocks, id))
				unlocks[static_cast<std::size_t>(unlock->typeId) - 1].emplace_back(id);
		}

		for (auto& ids : unlocks)
		{
			std::sort(std::begin(ids), std::end(ids));
			ids.erase(std::unique(std::begin(ids), std::end(ids)), std::end(ids));
		}
		return unlocks;
	}

	game::IDs _extractSkins(const json::Object& _json)
	{
		auto& jsonUnlocks = json::getArray(_json, "skins");
		game::IDs ids;
		ids.reserve(std::size(jsonUnlocks));
		std::transform(std::begin(jsonUnlocks), std::end(jsonUnlocks), std::back_inserter(ids),
			[](const auto& _value) { return _value.get<game::ID>(); }
		);
		return ids;
	}

	game::ParticipantData _createParticipantData(const json::Object& _json, GameDataPtr _gameData)
	{
		assert(_gameData);
		std::optional<game::ID> lastPage;
		if (auto lastPagePtr = json::getPtr<json::UInt>(_json, "lastPage"))
			lastPage = static_cast<game::ID>(*lastPagePtr);
		return { game::ParticipantData::Human{ json::getValue<game::ID>(_json, "uid") }, json::getValue<std::string>(_json, "name"), json::getValue<game::ID>(_json, "playerIcon"),
			_extractSkins(_json), _extractUnlocks(_json, *_gameData), json::getValue<game::ID>(_json, "color"), _readModPages(_json), lastPage };
	}

	game::MatchPtr _createMatch(const session::SessionPtrs& _sessions, const PreparationState& _prepState, const Definition& _matchDef, const GameDataPtr& _gameData)
	{
		game::MatchDef def;
		def.readableMatchType = get<DefinitionIndex::type>(_matchDef);
		def.tempId = get<DefinitionIndex::tmpId>(_matchDef);
		def.gameMode = game::MatchDef::GameMode::normal;
		def.pickMode = game::MatchDef::PickMode::normal;

		def.participants.reserve(std::size(_sessions));
		std::transform(std::begin(_sessions), std::end(_sessions), std::back_inserter(def.participants),
			[&_gameData, &playerDefinitions = _prepState.players] (const auto& _session)
			{
				assert(_session);
				auto def = _findPlayerDefinition(_session->getToken(), playerDefinitions);
				assert(def);
				return _session->createHandler<session::PlayerControllerHandler>(_createParticipantData(*def, _gameData));
			}
		);

		def.participants.insert(std::end(def.participants), std::begin(_prepState.bots), std::end(_prepState.bots));
		return game::createMatch(std::move(def), _gameData);
	}

	PreparationState _makePreparationState(const Definition& _def)
	{
		struct Visitor
		{
			PreparationState state;

			void operator()(const json::Object& _obj) { state.players.emplace_back(&_obj); }
			void operator()(int _botLevel) { state.bots.emplace_back(std::clamp(_botLevel, 0, 100)); }
		} visitor;

		for (auto& player : get<DefinitionIndex::playerDefinitions>(_def))
			std::visit(visitor, player);

		assert(0 < std::size(visitor.state.players));
		return visitor.state;
	}

	RunningState _makeRunningState(const session::SessionPtrs& _sessions, const PreparationState& _prepState,  const Definition& _matchDef, const GameDataPtr& _gameData)
	{
		RunningState state;
		state.worker = std::make_unique<Worker>(_matchDef, _createMatch(_sessions, _prepState, _matchDef, _gameData));
		state.thread = std::thread(
			[&worker = *state.worker]{ worker(); }
		);
		return state;
	}

	bool _exchangeSessionIfCan(const session::SessionPtr& _session, session::SessionPtrs& _sessions)
	{
		if (auto itr = std::find_if(std::begin(_sessions), std::end(_sessions),
			[token = _session->getToken()](const auto& _session) { return _session->getToken() == token; });
			itr != std::end(_sessions))
		{
			auto& session = **itr;
			session.setConnection(_session->getConnection());
			session.sendMessage(session::makeLogin(protobuf::out::Login_Result_reconnect));
			return true;
		}
		return false;
	};

	/*#####
	# Worker
	#####*/
	Worker::Worker(const Definition& _definition, game::MatchPtr _match) :
		m_Definition(_definition),
		m_Match(std::move(_match))
	{
		assert(m_Match);
	}

	bool Worker::_handleMatchStates(game::Match::State& _state)
	{
		if (m_ShallStop.load() || _state.type == game::Match::StateType::finished)
			return true;

		using namespace game::phase;
		struct Handler
		{
			const std::atomic<std::optional<Id>>& permaMatchIdAtomic;
			SendMatchStatistics& onSendMatchStatistics;
			bool& handledMatchStatistics;

			bool operator ()(std::monostate) { return false; }
			bool operator ()(GameState& _state) { return false; }
			bool operator ()(PickState& _state) { return false; }

			bool operator ()(FinalizeState& _state)
			{
				if (!handledMatchStatistics)
				{
					assert(_state.statistics);
					onSendMatchStatistics(*_state.statistics);
					handledMatchStatistics = true;
				}
				else if (auto permaMatchId = permaMatchIdAtomic.load())
				{
					assert(_state.permMatchId);
					*_state.permMatchId = permaMatchId;
				}
				return false;
			}
		};
		return std::visit(Handler{ m_PermaId, onSendMatchStatistics, m_HandledMatchStatistics }, _state.data);
	}

	void Worker::operator ()()
	{
		auto tmpId = get<DefinitionIndex::tmpId>(m_Definition);
		try
		{
			LOG_CHANNEL_INFO(hostLogger, "Start match thread tmpId: " << tmpId << " threadId: " << std::this_thread::get_id());
			using clock = std::chrono::steady_clock;
			auto last = clock::now();
			std::optional<game::Match::State> matchState;
			constexpr auto cycleDuration = std::chrono::milliseconds(50);
			while (!matchState || !_handleMatchStates(*matchState))
			{
				auto begin = clock::now();
				assert(m_Match);
				matchState = m_Match->update(std::chrono::duration_cast<game::Time>(begin - last));
				auto elapsed = clock::now() - begin;
				
				if (auto sleepDuration = cycleDuration - elapsed; game::Time::zero() < sleepDuration)
					std::this_thread::sleep_for(sleepDuration);
				last = begin;
			}
			LOG_CHANNEL_INFO(hostLogger, "Stopping match thread tmpId: " << tmpId << " threadId: " << std::this_thread::get_id());
		}
		catch (const std::exception& _e)
		{
			LOG_CHANNEL_FATAL(hostLogger, "Caught exception in match thread tmpId: " << tmpId << " threadId: " << std::this_thread::get_id() << ": " << _e.what());
		}
		catch (...)
		{
			LOG_CHANNEL_FATAL(hostLogger, "Caught exception in match thread tmpId: " << tmpId << " threadId: " << std::this_thread::get_id() << ": unspecified exception");
		}
		m_IsActive = false;
	}

	void Worker::setPermaId(Id _id)
	{
		if (!m_PermaId.load())
		{
			LOG_CHANNEL_INFO(hostLogger, "MatchWorker: Match tmpId: " << get<DefinitionIndex::tmpId>(m_Definition) << " received permId: " << _id);
			m_PermaId = _id;
		}
		else
			LOG_CHANNEL_ERR(hostLogger, "MatchWorker: Match tmpId: " << get<DefinitionIndex::tmpId>(m_Definition) << " already received permId: " << *m_PermaId.load() << " rejected new permaId: " << _id);
	}

	bool Worker::isActive() const
	{
		return m_IsActive;
	}

	void Worker::stop()
	{
		m_ShallStop = true;
	}

	/*#####
	# Handler
	#####*/
	Handler::Handler(Definition _def, GameDataPtr _gameData) :
		m_MatchDef(std::move(_def)),
		m_GameData(std::move(_gameData)),
		m_StateVariables(_makePreparationState(m_MatchDef))
	{
	}

	Handler::~Handler()
	{
		if (auto runState = std::get_if<RunningState>(&m_StateVariables))
		{
			runState->worker->stop();
			if (runState->thread.joinable())
				runState->thread.join();
		}
		LOG_CHANNEL_INFO(hostLogger, "Match tmpId: " << getTmpId(*this) << " finished.");
	}

	const match::Definition& Handler::getDefinition() const noexcept
	{
		return m_MatchDef;
	}

	std::chrono::system_clock::time_point Handler::getStartTime() const
	{
		return m_StartTimePoint;
	}

	Handler::State Handler::getState() const noexcept
	{
		std::scoped_lock<std::mutex> lock(m_StateVariablesMx);
		if (std::holds_alternative<PreparationState>(m_StateVariables))
			return State::preparation;
		if (auto runningState = std::get_if<RunningState>(&m_StateVariables))
		{
			if (runningState->worker->isActive())
				return State::running;
		}
		return State::finished;
	}

	bool Handler::addAsParticipant(session::SessionPtr _session)
	{
		if (!_session)
			throw std::runtime_error("MatchMgr::addSessionToMatch: detected null session");

		std::scoped_lock<std::mutex> lock(m_StateVariablesMx);
		if (auto prepState = std::get_if<PreparationState>(&m_StateVariables))
		{
			if (auto playerDef = _findPlayerDefinition(_session->getToken(), prepState->players))
			{
				std::scoped_lock<std::mutex> lock(m_ParticipantsMx);
				if (!_exchangeSessionIfCan(_session, m_Participants))
				{
					m_Participants.emplace_back(std::move(_session));
					m_Participants.back()->sendMessage(session::makeLogin(protobuf::out::Login_Result_success));
					if (std::size(m_Participants) == std::size(prepState->players))
					{
						m_StateVariables = _makeRunningState(m_Participants, *prepState, m_MatchDef, m_GameData);
						auto& runState = std::get<RunningState>(m_StateVariables);
						runState.worker->onSendMatchStatistics.connect(
							[&onSendMatchStatistics = onSendMatchStatistics](auto& _stats) { onSendMatchStatistics(std::move(_stats)); }
						);
					}
				}
				return true;
			}
		}
		else
		{
			std::scoped_lock<std::mutex> lock(m_ParticipantsMx);
			return _exchangeSessionIfCan(_session, m_Participants);
		}
		return false;
	}

	void Handler::sendMessageToParticipants(protobuf::out::Message _msg)
	{
		auto strMsg = _msg.SerializeAsString();
		std::scoped_lock<std::mutex> lock(m_ParticipantsMx);
		for (auto& session : m_Participants)
			session->sendMessage(strMsg);
	}

	void Handler::setPermaId(Id _permaId)
	{
		std::scoped_lock<std::mutex> lock(m_StateVariablesMx);
		if (auto runningState = std::get_if<RunningState>(&m_StateVariables))
		{
			runningState->worker->setPermaId(_permaId);
		}
	}

	//bool Handler::addAsSpectator(session::SessionPtr _session)
	//{

	//}

	bool isActive(const Handler& _match)
	{
		auto state = _match.getState();
		return state == Handler::State::preparation ||
			state == Handler::State::running;
	}

	Id getTmpId(const Handler& _match)
	{
		return get<DefinitionIndex::tmpId>(_match.getDefinition());
	}

	/*#####
	# Manager
	#####*/
	Manager::Manager(const Config& _config) :
		m_Config(_config)
	{
	}

	void Manager::setPermMatchId(Id _tmpId, Id _permId)
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		if (auto itr = std::find_if(std::begin(m_Matches), std::end(m_Matches),
			[_tmpId](auto& _match) { return getTmpId(*_match) == _tmpId; });
			itr != std::end(m_Matches))
		{
			(*itr)->setPermaId(_permId);
		}
		else
			LOG_CHANNEL_ERR(hostLogger, "Unable to find Match tmpId: " << _tmpId << " to provide with permaId: " << _permId);
	}

	void Manager::setGameData(GameDataPtr _gameData)
	{
		assert(_gameData);
		std::atomic_store(&m_GameData, _gameData);
	}

	void Manager::addMatch(Definition _matchDef)
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		if (std::end(m_Matches) != std::find_if(std::begin(m_Matches), std::end(m_Matches),
			[&_matchDef](auto& _other) { return get<DefinitionIndex::tmpId>(_matchDef) == getTmpId(*_other); }
		))
		{
			throw std::runtime_error("MatchMgr: match with tmpId: " + std::to_string(get<DefinitionIndex::tmpId>(_matchDef)) + " is already registered.");
		}
		m_Matches.emplace_back(std::make_unique<Handler>(std::move(_matchDef), std::atomic_load(&m_GameData)));
		m_Matches.back()->onSendMatchStatistics.connect(
			[&onSendMatchStatistics = onSendMatchStatistics](auto& _stats) { onSendMatchStatistics(std::move(_stats)); }
		);
	}

	std::optional<Id> Manager::_tryAddSessionToMatch(session::SessionPtr _session)
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		for (auto& match : m_Matches)
		{
			auto state = match->getState();
			if (match->addAsParticipant(_session))
			{
				auto tmpId = getTmpId(*match);
				auto newState = match->getState();
				if (newState == Handler::State::running && newState != state)
					onMatchStarted(tmpId);
				return tmpId;
			}
		}
		return std::nullopt;
	}

	void Manager::addSessionToMatch(session::SessionPtr _session)
	{
		
		if (auto optTmpId = _tryAddSessionToMatch(_session))
		{
			LOG_CHANNEL_INFO(hostLogger, "MatchMgr: registered session sid: " << _session->getToken() << " at match tmpId: " << *optTmpId << " as participant.");
		}
		//else if (auto itr = std::find_if(std::begin(m_Matches), std::end(m_Matches),
		//	[&_session](auto& _match) { return _match.addAsSpectator(_session); }); itr != std::end(m_Matches))
		//{
		//	auto tmpId = get<DefinitionIndex::tmpId>(itr->getDefinition());
		//	LOG_CHANNEL_INFO(hostLogger, "MatchMgr: registered session sid: " << _session->getToken() << " at match tmpId: " << tmpId << " as spectator.");
		//	return;
		//}
		else
		{
			_session->sendMessage(session::makeLogin(protobuf::out::Login_Result_noMatchFound));
			throw std::runtime_error("MatchMgr: no related match found.");
		}
	}

	bool Manager::hasMatch(Id _tmpId) const
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		return std::end(m_Matches) != std::find_if(std::begin(m_Matches), std::end(m_Matches),
			[_tmpId](const auto& _match) { return getTmpId(*_match) == _tmpId; }
		);
	}

	std::vector<MatchInformation> Manager::gatherMatchInformations() const
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		std::vector<MatchInformation> infos;
		infos.reserve(std::size(m_Matches));

		for (auto& match : m_Matches)
		{
			if (isActive(*match))
				infos.emplace_back(MatchInformation{ match->getDefinition(), match->getState() });
		}
		return infos;
	}

	void Manager::cleanup()
	{
		std::scoped_lock<std::mutex> lock(m_MatchesMx);
		m_Matches.erase(std::remove_if(std::begin(m_Matches), std::end(m_Matches),
			[
				maxPrepDuration = m_Config.prepareMatchesMaxDuration,
				&onMatchCanceled = onMatchCanceled,
				now = std::chrono::system_clock::now()
			]
			(auto& _match)
			{
				switch (_match->getState())
				{
				case Handler::State::finished:
					LOG_CHANNEL_INFO(hostLogger, "Match tmpId: " << getTmpId(*_match) << " is in closing state. Start deletion afterwards.");
					return true;
				case Handler::State::preparation:
					if (maxPrepDuration < now - _match->getStartTime())
					{
						_match->sendMessageToParticipants(session::makeLogin(protobuf::out::Login_Result_timeout));
						onMatchCanceled(getTmpId(*_match));
						LOG_CHANNEL_INFO(hostLogger, "Cancel match tmpId: " << getTmpId(*_match) << ". Preperation time has been exceeded.");
						return true;
					}
					break;
				}
				return false;
			}
		), std::end(m_Matches));
	}
} // namespace match
