#pragma once

#include "MatchMgr.hpp"
#include "network/Device.hpp"
#include "network/Connection.hpp"
#include "session/Factory.hpp"
#include "TimerFactory.hpp"

class Host
{
public:
	Host(const Config& _config);

	void run();

private:
	const Config& m_Config;
	network::Device m_NetworkDevice;
	TimerFactory m_TimerFactory;
	match::Manager m_MatchMgr;
	session::Factory m_SessionFactory;
	network::ConnectionPtr m_HomeConnection;

	void _setupHomeConnection(network::ConnectionPtr _con);
	void _setup();
	void _setupMatchMgr();
	void _matchFinished(const game::Statistics& _gameStats);
};
