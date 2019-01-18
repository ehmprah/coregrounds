#include "stdafx.h"
#include "Factory.hpp"
#include "DurationalObject.hpp"

namespace session
{
	Factory::Factory(TimerFactory& _timerFactory) :
		m_TimerFactory(_timerFactory)
	{
	}

	std::shared_ptr<UnregSession> Factory::createUnregSession(network::ConnectionPtr _connection)
	{
		auto obj = std::make_shared<DurationalObject<std::shared_ptr<UnregSession>>>(std::make_shared<UnregSession>(std::move(_connection)));
		(**obj)->setupSelf();
		obj->setupTimer(m_TimerFactory);
		return **obj;
	}

	template <class TSessions>
	auto _findSession(const TSessions& _sessions, session::TokenView _token)
	{
		auto findByToken = [_token](const auto& _session) { return _session->getToken() == _token; };
		return std::find_if(std::begin(_sessions), std::end(_sessions), findByToken);
	}

	SessionPtr Factory::createSession(network::ConnectionPtr _connection, Token _token) const
	{
		try
		{
			auto newSession = std::make_shared<session::Session>(std::move(_token));
			newSession->setConnection(std::move(_connection));
			LOG_INFO("session::Factory::createSession: created session for user sid: " << newSession->getToken());
			return newSession;
		}
		catch (const std::exception& _e)
		{
			LOG_ERR("session::Factory::createSession: caught exception: " << _e.what());
		}
		return nullptr;
	}
} // namespace session
