#include "stdafx.h"
#include "Session.hpp"
#include "Factory.hpp"
#include "Handler.hpp"

namespace session
{
	protobuf::out::Message makeLogin(protobuf::out::Login::Result _result)
	{
		protobuf::out::Message msg;
		auto& login = *msg.mutable_login();
		login.set_result(_result);
		return msg;
	}

	/*#####
	# UnregSession
	#####*/
	UnregSession::UnregSession(network::ConnectionPtr _connection) :
		m_Connection(std::move(_connection))
	{
	}

	UnregSession::~UnregSession()
	{
		if (auto connection = std::atomic_load(&m_Connection))
		{
			connection->asyncWrite(makeLogin(protobuf::out::Login_Result_timeout).SerializeAsString(), true);
			connection->close();
			LOG_ERR("session::UnregSession: cancled on timeout");
		}
	}

	void UnregSession::setupSelf()
	{
		auto connection = std::atomic_load(&m_Connection);
		assert(connection);
		m_ReadCon = connection->onRead.connect(decltype(connection->onRead)::slot_type(
			[weakPtr = weak_from_this(), weakCon = connection->weak_from_this()](const auto& _buffer, bool _binary) {
				if (_binary)
				{
					if (auto ptr = weakPtr.lock())
					{
						try
						{
							protobuf::in::Message msg;
							if (msg.ParseFromArray(boost::asio::buffer_cast<char const*>(boost::beast::buffers_front(_buffer.data())), boost::asio::buffer_size(_buffer.data())))
								ptr->_tryRegister(std::move(msg));
						}
						catch (const std::exception& _e)
						{
							LOG_ERR("UnregSession::asyncRead: " << _e.what());
						}
						catch (...)
						{
							LOG_ERR("UnregSession::asyncRead: unspecified exception");
						}
					}
				}
			}
		).track_foreign(shared_from_this()));
	}

	void UnregSession::_tryRegister(protobuf::in::Message _msg)
	{
		if (!_msg.has_login() || _msg.login().token().empty())
			throw std::runtime_error("Received invalid message.");
		else if (auto connection = std::atomic_exchange(&m_Connection, network::ConnectionPtr()))
		{
			m_ReadCon.disconnect();
			onAuthentication(std::move(connection), _msg.login().token());
		}
	}

	Session::Session(Token _token) :
		m_Token(std::move(_token))
	{
	}

	void Session::setConnection(network::ConnectionPtr _connection)
	{
		m_SocketReadCon = _connection->onRead.connect(
			[weakptr = weak_from_this()](const auto& _buffer, bool _binary)
			{
				if (auto ptr = weakptr.lock())
					ptr->_onSocketRead(_buffer, _binary);
			}
		);
		m_SocketDisconnectCon = _connection->onDisconnect.connect(
			[weakptr = weak_from_this()]()
			{
				if (auto ptr = weakptr.lock())
					ptr->_onSocketDisconnect();
			}
		);

		std::atomic_store(&m_Connection, std::move(_connection));
		
		std::scoped_lock<std::mutex> lock(m_HandlerMx);
		if (m_Handler)
			m_Handler->connected();
	}

	network::ConnectionPtr Session::getConnection() const
	{
		return std::atomic_load(&m_Connection);
	}

	void Session::sendMessage(const protobuf::out::Message& _msg)
	{
		sendMessage(_msg.SerializeAsString());
	}

	void Session::sendMessage(std::string _msg)
	{
		if (auto connection = std::atomic_load(&m_Connection))
			connection->asyncWrite(std::move(_msg), true);
	}

	TokenView Session::getToken() const
	{
		assert(!m_Token.empty());
		return m_Token;
	}

	void Session::_onSocketDisconnect()
	{
		LOG_INFO("Session: Socket disconnected.");
		onClose();

		std::scoped_lock<std::mutex> lock(m_HandlerMx);
		if (m_Handler)
			m_Handler->disconnected();
	}

	void Session::_onSocketRead(const network::Connection::ReadBuffer& _buffer, bool _binary)
	{
		if (_binary)
		{
			protobuf::in::Message msg;
			if (msg.ParseFromArray(boost::asio::buffer_cast<char const*>(boost::beast::buffers_front(_buffer.data())), boost::asio::buffer_size(_buffer.data())))
			{
				std::scoped_lock<std::mutex> lock(m_HandlerMx);
				if (m_Handler)
					m_Handler->handleEvent(std::move(msg));
			}
		}
	}
} // namespace session
