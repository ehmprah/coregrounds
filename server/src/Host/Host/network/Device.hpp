#pragma once

#include "Host/Config.hpp"
#include "Connection.hpp"

namespace network
{
	class Device :
		sl2::NonCopyable
	{
	public:
		using IncConnectionCB = std::function<void(ConnectionPtr)>;

		Device(const Config& _config);
		~Device();

		void run();
		void shutdown();

		ConnectionPtr connect(tcp::endpoint _endpoint);

		boost::asio::io_context& getIOContext();
		void setConnectionCallback(IncConnectionCB _cb);

	private:
		class Listener
		{
		public:
			Listener(boost::asio::io_context& _ioContext, tcp::endpoint _ep, std::function<void(tcp::socket&&)> _addConnectionCallback);

			void run();
			void shutdown();

		private:
			tcp::acceptor m_Acceptor;
			tcp::socket m_Socket;
			std::function<void(tcp::socket&&)> m_AddConnectionCallback;

			void _onAccept(boost::system::error_code _ec);
		};

		const Config& m_Config;

		boost::asio::io_context m_IOContext;
		std::list<std::atomic_bool> m_IOThreadStop;
		std::vector<std::thread> m_IOContextThreads;
		tcp::resolver m_TcpResolver;
		Listener m_Listener;

		std::mutex m_IncConMx;
		/*! \brief incoming connection callback
		/*	\details It is guaranteed, to only emit once a time.
		*/
		IncConnectionCB m_IncConCB;

		void _addConnection(tcp::socket&& _socket);
	};
} // namespace network
