#pragma once

#include "Session.hpp"
#include "TimerFactory.hpp"

namespace session
{
	class Factory :
		sl2::NonCopyable
	{
	public:
		Factory(TimerFactory& _timerFactory);

		std::shared_ptr<UnregSession> createUnregSession(network::ConnectionPtr _connection);
		SessionPtr createSession(network::ConnectionPtr _connection, Token _token) const;

	private:
		TimerFactory& m_TimerFactory;
	};
} // namespace session
