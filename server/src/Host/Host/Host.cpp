#include "stdafx.h"
#include "Host.hpp"
#include "Game/DataReader.hpp"
#include "Log.hpp"

json::Object _createJsonDocument(std::string_view _data)
{
	return json::Object::parse(_data);
}

std::shared_ptr<game::data::Game> _readGameData(const json::Object& _doc)
{
#ifndef LOCAL
	game::data::JsonReader reader(json::getElement(_doc, "data"));
#else
	std::ifstream in("server_config.json");
	json::Object doc;
	in >> doc;
	game::data::JsonReader reader(doc);
#endif
	return reader.read();
}

json::Object _makeMatchEvent(std::string_view _event, match::Id _tmpMatchId)
{
	json::Object doc;
	doc["event"] = _event.data();
	doc["data"]["matchId"] = _tmpMatchId;
	return doc;
}

void _sendMatchEvent(network::Connection& _connection, std::string_view _event, match::Id _tmpMatchId)
{
	auto doc = _makeMatchEvent(_event, _tmpMatchId);
	auto dump = doc.dump();
	_connection.asyncWrite(dump);
	LOG_CHANNEL_INFO(network::outLogger, dump);
}

void _sendGameServerReady(network::Connection& _connection)
{
	constexpr auto msg = "{\"event\": \"GameServerReady\" }";
	_connection.asyncWrite(msg);
	LOG_CHANNEL_INFO(network::outLogger, msg);
}

void _sendMatchConfirm(network::Connection& _connection, match::Id _tmpMatchId)
{
	_sendMatchEvent(_connection, "GameServerGameConfirm", _tmpMatchId);
}

void _sendMatchRefuse(network::Connection& _connection, match::Id _tmpMatchId)
{
	_sendMatchEvent(_connection, "GameServerGameRefuse", _tmpMatchId);
}

void _sendMatchCancel(network::Connection& _connection, match::Id _tmpMatchId)
{
	LOG_CHANNEL_INFO(hostLogger, "Begin send match cancel tempId: " << _tmpMatchId);
	_sendMatchEvent(_connection, "GameServerGameCancel", _tmpMatchId);
	LOG_CHANNEL_INFO(hostLogger, "Finished send match cancel tempId: " << _tmpMatchId);
}

void _sendMatchStart(network::Connection& _connection, match::Id _tmpMatchId)
{
	_sendMatchEvent(_connection, "GameServerMatchStart", _tmpMatchId);
}

void _createMatch(const json::Object& _doc, match::Manager& _matchMgr, network::Connection& _connection)
{
	auto& data = _doc.at("data");
	auto matchId = json::getValue<int>(data, "matchId");
	try
	{
		LOG_CHANNEL_INFO(hostLogger, "Begin match preparation");
		std::vector<match::PlayerDef> players;
		for (auto& el : json::getArray(data, "players"))
		{
			if (auto val = json::getPtr<std::string>(el, "type"); val && *val == "bot")
				players.emplace_back(json::getValue<int>(el, "level"));
			else
				players.emplace_back(el);
		}
		_matchMgr.addMatch({ matchId, json::getValue<std::string>(data, "token"),
			json::getValue<std::string>(data, "type"), std::move(players) });

		LOG_CHANNEL_INFO(hostLogger, "Finished match preparation tempId: " << matchId);
		_sendMatchConfirm(_connection, matchId);
		return;
	}
	catch (const std::exception& _e)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during match preparation: tmpId: " << matchId << ":" << _e.what());
	}
	catch (...)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during match preparation:  tmpId: " << matchId << ": unspecified error");
	}
	_sendMatchRefuse(_connection, matchId);
}

void _replyToHeartBeatGame(const json::Object& _doc, match::Manager& _matchMgr, network::Connection& _connection)
{
	try
	{
		LOG_CHANNEL_INFO(hostLogger, "Begin match heartbeat reply");
		auto& data = json::getElement(_doc, "data");
		auto matchId = json::getValue<int>(data, "matchId");

		json::Object doc;
		doc["event"] = "GameServerHeartBeatGame";
		auto& outData = doc["data"];
		outData["matchId"] = matchId;
		auto running = _matchMgr.hasMatch(matchId);
		outData["running"] = running;

		auto dump = doc.dump();
		_connection.asyncWrite(dump);
		LOG_CHANNEL_INFO(network::outLogger, dump);
		LOG_CHANNEL_INFO(hostLogger, "Finished match heartbeat reply for tempId: " << matchId << " running?: " << running);
	}
	catch (const std::exception& _e)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during match heartbeat reply: " << _e.what());
	}
	catch (...)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during match heartbeat reply: unspecified error");
	}
}

void _setupPermMatchId(const json::Object& _doc, match::Manager& _matchMgr)
{
	try
	{
		LOG_CHANNEL_INFO(hostLogger, "Begin set match permaId");
		auto& data = json::getElement(_doc, "data");
		_matchMgr.setPermMatchId(json::getValue<game::ID>(data, "matchId"), json::getValue<game::ID>(data, "permanentId"));
		LOG_CHANNEL_INFO(hostLogger, "Finished set match permaId");
	}
	catch (const std::exception& _e)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during set match permaId: " << _e.what());
	}
	catch (...)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during set match permaId: unspecified error");
	}
}

void _sendMatchList(network::Connection& _con, const match::Manager& _matchMgr)
{
	LOG_CHANNEL_INFO(hostLogger, "Begin send match list");
	json::Object doc;
	doc["event"] = "GameServerMatchList";
	auto& data = doc["data"];
	auto& matches = data["matches"];

	auto matchDefinitions = _matchMgr.gatherMatchInformations();
	std::transform(std::begin(matchDefinitions), std::end(matchDefinitions), std::back_inserter(matches),
		[](auto& _info)
		{
			json::Object obj;
			obj["state"] = static_cast<int>(_info.state);
			obj["matchId"] = match::get<match::DefinitionIndex::tmpId>(_info.definition);
			obj["type"] = match::get<match::DefinitionIndex::type>(_info.definition);
			obj["token"] = match::get<match::DefinitionIndex::token>(_info.definition);
			auto& players = obj["players"];
			for (auto& playerDef : match::get<match::DefinitionIndex::playerDefinitions>(_info.definition))
			{
				if (std::holds_alternative<json::Object>(playerDef))
					players.emplace_back(json::getValue<game::ID>(std::get<json::Object>(playerDef), "uid"));
			}
			return obj;
		}
	);

	auto dump = doc.dump();
	_con.asyncWrite(dump);
	LOG_CHANNEL_INFO(network::outLogger, dump);
	LOG_CHANNEL_INFO(hostLogger, "Finished send match list");
}

void _handleNetworkEvents(const json::Object& _doc, match::Manager& _matchMgr, network::Connection& _connection)
{
	if (auto event = json::getElementPtr(_doc, "event"))
	{
		if (*event == "GameServerSetConfig")
		{
			LOG_CHANNEL_INFO(hostLogger, "Begin processing game config");
			_matchMgr.setGameData(_readGameData(_doc));
			LOG_CHANNEL_INFO(hostLogger, "Finished processing game config");
			_sendGameServerReady(_connection);
		}
		else if (*event == "GameServerGetMatchList")
		{
			_sendMatchList(_connection, _matchMgr);
		}
		else if (*event == "GameServerGameCreate")
		{
			_createMatch(_doc, _matchMgr, _connection);
		}
		else if (*event == "GameServerSendMatchId")
		{
			_setupPermMatchId(_doc, _matchMgr);
		}
		else if (*event == "GameServerHeartBeatGame")
		{
			_replyToHeartBeatGame(_doc, _matchMgr, _connection);
		}
		else
			LOG_ERR("Host: received unknown event: " << event);
	}
	else
		LOG_ERR("Host: received text message without event type.");
}

network::ConnectionPtr _createHomeConnection(const Config& _config, network::Device& _networkDevice)
{
	auto connection = _networkDevice.connect({ boost::asio::ip::address::from_string(_config.homeIp), _config.homePort });
	assert(connection);
	return connection;
}

void _sendMatchStats(network::Connection& _con, game::Statistics _stats)
{
	LOG_CHANNEL_INFO(hostLogger, "Begin send match stats for tempId: " << _stats.tempMatchId);
	json::Object doc;
	doc["event"] = "GameServerGameFinished";
	auto& data = doc["data"];
	data["matchId"] = _stats.tempMatchId;
	data["match"] = _stats.toJson();

	auto dump = doc.dump();
	_con.asyncWrite(dump);
	LOG_CHANNEL_INFO(network::outLogger, dump);
	LOG_CHANNEL_INFO(hostLogger, "Finished send match stats for tempId: " << _stats.tempMatchId);
}

void _handleAuthenticatedConnection(const session::Factory& _sessionFactory, match::Manager& _matchMgr, network::ConnectionPtr _con, session::Token _token)
{
	try
	{
		auto newSession = _sessionFactory.createSession(std::move(_con), std::move(_token));
		assert(newSession);
		_matchMgr.addSessionToMatch(newSession);
	}
	catch (const std::exception& _e)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during handling authenticated session: " << _e.what());
	}
	catch (...)
	{
		LOG_CHANNEL_ERR(hostLogger, "Caught exception during handling authenticated session: unspecified error");
	}
}

/*#####
# Host
#####*/
Host::Host(const Config& _config) :
	m_Config(_config),
	m_NetworkDevice(_config),
	m_TimerFactory(m_NetworkDevice.getIOContext()),
	m_MatchMgr(_config),
	m_SessionFactory(m_TimerFactory)
{
	m_NetworkDevice.setConnectionCallback(
		[&sessionFactory = m_SessionFactory, &matchMgr = m_MatchMgr](network::ConnectionPtr _con)
		{
			auto unregSession = sessionFactory.createUnregSession(std::move(_con));
			assert(unregSession);
			unregSession->onAuthentication.connect(
				[&sessionFactory, &matchMgr](auto _con, auto _token)
				{
					_handleAuthenticatedConnection(sessionFactory, matchMgr, std::move(_con), _token.data());
				}
			);
		}
	);
	m_NetworkDevice.run();

	LOG_CHANNEL_INFO(hostLogger, "Begin setup lobby server connection to: " << m_Config.homeIp << ":" << m_Config.homePort);
	_setupHomeConnection(_createHomeConnection(m_Config, m_NetworkDevice));
	LOG_CHANNEL_INFO(hostLogger, "Finished setup lobby connection");

	_setupMatchMgr();
}

void Host::run()
{
	for(;; std::this_thread::sleep_for(std::chrono::seconds(1)))
	{
		try
		{
			m_MatchMgr.cleanup();
		}
		catch (const std::exception& _e)
		{
			LOG_CHANNEL_ERR(hostLogger, "Caught exception during execution of interval jobs: " << _e.what());
		}
		catch (...)
		{
			LOG_CHANNEL_ERR(hostLogger, "Caught exception during execution of interval jobs: unspecified error");
		}
	}
}

void Host::_setupMatchMgr()
{
	m_MatchMgr.onSendMatchStatistics.connect(
		[&con = m_HomeConnection](const auto& _matchStatistics)
		{
			if (auto conCopy = std::atomic_load(&con))
				_sendMatchStats(*conCopy, _matchStatistics);
		}
	);

	m_MatchMgr.onMatchCanceled.connect(
		[&con = m_HomeConnection](match::Id _tmpId)
		{
			if (auto conCopy = std::atomic_load(&con))
				_sendMatchCancel(*conCopy, _tmpId);
		}
	);

	m_MatchMgr.onMatchStarted.connect(
		[&con = m_HomeConnection](match::Id _tmpId)
		{
			if (auto conCopy = std::atomic_load(&con))
				_sendMatchStart(*conCopy, _tmpId);
		}
	);
}

void Host::_matchFinished(const game::Statistics& _gameStats)
{
	json::Object doc;
	doc["event"] = "GameServerGameFinished";
	auto& data = doc["data"];
	data["matchId"] = _gameStats.tempMatchId;
	data["match"] = _gameStats.toJson();

	auto dump = doc.dump();
	assert(m_HomeConnection);
	m_HomeConnection->asyncWrite(dump);
	LOG_CHANNEL_INFO(network::outLogger, dump);
}

void Host::_setupHomeConnection(network::ConnectionPtr _con)
{
	assert(_con);
	if (!_con->isOpen())
		return;

	_con->onRead.connect(
		[weakHome = _con->weak_from_this(), &matchMgr = m_MatchMgr](auto& _buffer, bool _binary)
		{
			LOG_CHANNEL_INFO(hostLogger, "Host received network message: binary?: " << _binary);
			if (auto homeConnection = weakHome.lock();
				homeConnection && !_binary)
			{
				try
				{
					std::string_view data(boost::asio::buffer_cast<char const*>(boost::beast::buffers_front(_buffer.data())), boost::asio::buffer_size(_buffer.data()));
					LOG_CHANNEL_INFO(network::inLogger, data);
					_handleNetworkEvents(_createJsonDocument(data), matchMgr, *homeConnection);
				}
				catch (const std::exception& _e)
				{
					LOG_CHANNEL_ERR(hostLogger, "Caught exception during input processing: " << _e.what());
				}
				catch (...)
				{
					LOG_CHANNEL_ERR(hostLogger, "Caught exception during input processing: unspecified error");
				}
			}
		}
	);

	_con->onDisconnect.connect(
		[weakHome = _con->weak_from_this(), this]()
		{
			if (auto homeConnection = weakHome.lock())
			{
				bool success = false;
				while (!success)
				{
					try
					{
						LOG_CHANNEL_INFO(hostLogger, "Begin reconnecting to lobby sever");
						auto newCon = _createHomeConnection(m_Config, m_NetworkDevice);
						assert(newCon);
						_setupHomeConnection(newCon);
						success = true;
						LOG_CHANNEL_INFO(hostLogger, "Finished reconnecting to lobby sever");
					}
					catch (const std::exception& _e)
					{
						LOG_CHANNEL_ERR(hostLogger, "Caught exception during reconnecting to lobby server: " << _e.what());
						success = false;
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
					catch (...)
					{
						LOG_CHANNEL_ERR(hostLogger, "Caught exception during reconnecting to lobby server: unspecified error");
						success = false;
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
				}
			}
		}
	);

	LOG_CHANNEL_INFO(hostLogger, "Established lobby connection");
	std::atomic_store(&m_HomeConnection, std::move(_con));

	// notify lobby about new connection
	json::Object doc;
	doc["event"] = "GameServerRegister";
	auto dump = doc.dump();
	LOG_CHANNEL_INFO(network::outLogger, dump);
	m_HomeConnection->asyncWrite(dump);
}
