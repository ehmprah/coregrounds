#pragma once

#include "network/Connection.hpp"

namespace session
{
	class Factory;

	using UID = uint64_t;
	using TokenView = std::string_view;
	using Token = std::string;

	class UnregSession :
		public std::enable_shared_from_this<UnregSession>,
		sl2::NonCopyable
	{
	public:
		UnregSession(network::ConnectionPtr _connection);
		~UnregSession();

		void setupSelf();

		using AuthenticationSignal = boost::signals2::signal<void(network::ConnectionPtr, TokenView)>;
		AuthenticationSignal onAuthentication;

	private:
		network::ConnectionPtr m_Connection;
		boost::signals2::scoped_connection m_ReadCon;

		void _msgReceived();
		void _tryRegister(protobuf::in::Message _msg);
	};

	class AbstractHandler;

	class Session :
		public std::enable_shared_from_this<Session>
	{
	public:
		Session(Token _token);

		void setConnection(network::ConnectionPtr _connection);
		network::ConnectionPtr getConnection() const;

		void sendMessage(const protobuf::out::Message& _msg);
		void sendMessage(std::string _msg);

		TokenView getToken() const;

		template <class T, typename... Args>
		std::weak_ptr<T> createHandler(Args&&... _args)
		{
			static_assert(std::is_base_of_v<AbstractHandler, T>);
			auto ptr = std::make_shared<T>(std::forward<Args>(_args)...);
			ptr->onWrite.connect(
				[weakptr = weak_from_this()](const protobuf::out::Message& _msg)
				{
					if (auto ptr = weakptr.lock())
						ptr->sendMessage(_msg);
				}
			);

			std::scoped_lock<std::mutex> lock(m_HandlerMx);
			m_Handler = ptr;
			return ptr;
		}

		using CloseSignal = boost::signals2::signal<void()>;
		CloseSignal onClose;

	private:
		Token m_Token;
		network::ConnectionPtr m_Connection;
		
		std::mutex m_HandlerMx;
		std::shared_ptr<AbstractHandler> m_Handler;

		boost::signals2::scoped_connection m_SocketReadCon;
		boost::signals2::scoped_connection m_SocketDisconnectCon;
		void _onSocketDisconnect();

		void _onSocketRead(const network::Connection::ReadBuffer& _buffer, bool _binary);
	};
	using SessionPtr = std::shared_ptr<Session>;
	using SessionPtrs = std::vector<SessionPtr>;

	protobuf::out::Message makeLogin(protobuf::out::Login::Result _result);
} // namespace session
