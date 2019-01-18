#include "stdafx.h"
#include "Device.hpp"

namespace network
{
	BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(DeviceLogger, ChannelLogger, (boost::log::keywords::channel = "network-device"));

	inline ChannelLogger& deviceLogger{ DeviceLogger::get() };

	Device::Listener::Listener(IOService& _ioContext, tcp::endpoint _ep, std::function<void(tcp::socket&&)> _addConnectionCallback) :
		m_Acceptor(_ioContext),
		m_Socket(_ioContext),
		m_AddConnectionCallback(_addConnectionCallback)
	{
		boost::system::error_code ec;

		LOG_CHANNEL_INFO(deviceLogger, "open TCP acceptor");
		if (m_Acceptor.open(_ep.protocol(), ec))
			throw Error("network::Device: " + ec.message());

		LOG_CHANNEL_INFO(deviceLogger, "bind TCP acceptor to local endpoint: " << _ep);
		if (m_Acceptor.bind(_ep, ec))
			throw Error("network::Device: " + ec.message());

		LOG_CHANNEL_INFO(deviceLogger, "TCP acceptor start listening");
		if (m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec))
			throw Error("network::Device: " + ec.message());

		LOG_CHANNEL_INFO(deviceLogger, "TCP acceptor successfully initialized");
	}

	void Device::Listener::_onAccept(boost::system::error_code _ec)
	{
		try
		{
			if (_ec)
				LOG_CHANNEL_ERR(deviceLogger, "onAccept: " << _ec);
			else
				m_AddConnectionCallback(std::move(m_Socket));
			
		}
		catch (const std::exception& _e)
		{
			LOG_CHANNEL_ERR(deviceLogger, "Caught exception during TCP Acceptor processing: \"" << _e.what() << "\" Try restarting TCP acceptor.");
		}
		catch (...)
		{
			LOG_CHANNEL_ERR(deviceLogger, "Caught exception during TCP Acceptor processing: unspecified error. Try restarting TCP acceptor.");
		}
		run();
	}

	void Device::Listener::run()
	{
		if (m_Acceptor.is_open())
		{
			LOG_CHANNEL_INFO(deviceLogger, "TCP acceptor listening for async connection");
			m_Acceptor.async_accept(m_Socket, std::bind(&Listener::_onAccept, this, std::placeholders::_1));
		}
		else
			LOG_CHANNEL_ERR(deviceLogger, "TCP acceptor unable to listen for async connection: Acceptor is not open.");
	}

	void Device::Listener::shutdown()
	{
		LOG_CHANNEL_INFO(deviceLogger, "Shutdown TCP acceptor");
		m_Acceptor.close();
	}

	/*#####
	# Device
	#####*/
	Device::Device(const Config& _config) :
		m_Config(_config),
		m_IOContext(std::max(1, _config.networkThreads)),
		m_TcpResolver(m_IOContext),
		m_Listener(m_IOContext, tcp::endpoint(boost::asio::ip::tcp::v4(), _config.networkPort), std::bind(&Device::_addConnection, this, std::placeholders::_1))
	{
	}

	Device::~Device()
	{
		try
		{
			shutdown();
		}
		catch (const std::exception& _e)
		{
			LOG_CHANNEL_FATAL(deviceLogger, "Destructor: " << _e.what());
		}
		catch (...)
		{
			LOG_CHANNEL_FATAL(deviceLogger, "Destructor: caught unspecified exception.");
		}
	}

	void Device::run()
	{
		m_Listener.run();

		auto threadCount = std::max(1, m_Config.networkThreads);
		LOG_CHANNEL_INFO(deviceLogger, "Begin starting " << threadCount << " network threads");
		if (threadCount != std::thread::hardware_concurrency())
			LOG_CHANNEL_WARN(deviceLogger, "Recommended network thread count: " << std::thread::hardware_concurrency());

		m_IOThreadStop.resize(threadCount);
		for (std::size_t i = 0; i < threadCount; ++i)
		{
			m_IOContextThreads.emplace_back(
				[&ioContext = m_IOContext, &stop = m_IOThreadStop.back(), index = i]
				{
					LOG_CHANNEL_INFO(deviceLogger, "Starting network thread index: " << index << " threadId: " << std::this_thread::get_id());
					while (!stop)
					{
						try
						{
							ioContext.run();
						}
						catch (const std::exception& _e)
						{
							LOG_CHANNEL_ERR(deviceLogger, "Caught exception during network thread execution: index: " << index <<
								" threadId: " << std::this_thread::get_id() << " error: " << _e.what());
						}
						catch (...)
						{
							LOG_CHANNEL_ERR(deviceLogger, "Caught exception during network thread execution: index: " << index << 
								" threadId: " << std::this_thread::get_id() << " unspecified error");
						}
						if (!stop)
							LOG_CHANNEL_INFO(deviceLogger, "Restarting network thread index: " << index << " threadId: " << std::this_thread::get_id());
					}
					LOG_CHANNEL_INFO(deviceLogger, "stopped network thread index: " << index << " threadId: " << std::this_thread::get_id());
				}
			);
		}
		assert(!std::empty(m_IOContextThreads) && std::size(m_IOContextThreads) == std::size(m_IOThreadStop));
		LOG_CHANNEL_INFO(deviceLogger, "Successfully started " << std::size(m_IOContextThreads) << " threads.");
	}

	void Device::shutdown()
	{
		LOG_CHANNEL_INFO(deviceLogger, "Begin shutdown");
		m_Listener.shutdown();

		LOG_CHANNEL_INFO(deviceLogger, "Begin stopping IO-context");
		for (auto& stop : m_IOThreadStop)
			stop = true;
		m_IOContext.stop();
		LOG_CHANNEL_INFO(deviceLogger, "Successfully stopped IO-context");

		LOG_CHANNEL_INFO(deviceLogger, "Begin joining network threads");
		for (auto& thread : m_IOContextThreads)
			thread.join();
		LOG_CHANNEL_INFO(deviceLogger, "Successfully joined network threads");
	}

	void Device::_addConnection(tcp::socket&& _socket)
	{
		LOG_CHANNEL_INFO(deviceLogger, "New incoming connection from: " << _socket.remote_endpoint());
		std::scoped_lock<std::mutex> lock(m_IncConMx);
		WebSocket ws(std::move(_socket));
		ws.accept();
		LOG_CHANNEL_INFO(deviceLogger, "Successfully initialized WebSocket connection with: " << ws.next_layer().remote_endpoint());
		assert(m_IncConCB);
		auto connection = std::make_shared<Connection>(std::move(ws));
		connection->exec();
		m_IncConCB(connection);
	}

	ConnectionPtr Device::connect(tcp::endpoint _endpoint)
	{
		LOG_CHANNEL_INFO(deviceLogger, "Begin connecting process with: " << _endpoint);
		auto results = m_TcpResolver.resolve(_endpoint);
		tcp::socket socket(m_IOContext);
		boost::asio::connect(socket, std::begin(results), std::end(results));
		LOG_CHANNEL_INFO(deviceLogger, "Successfully connected to: " << socket.remote_endpoint());
		WebSocket ws(std::move(socket));
		ws.handshake(_endpoint.address().to_string(), "/");
		LOG_CHANNEL_INFO(deviceLogger, "Successfully initialized WebSocket connection with: " << ws.next_layer().remote_endpoint());
		auto connection = std::make_shared<Connection>(std::move(ws));
		connection->exec();
		return connection;
	}

	boost::asio::io_context& Device::getIOContext()
	{
		return m_IOContext;
	}

	void Device::setConnectionCallback(IncConnectionCB _cb)
	{
		m_IncConCB = _cb;
	}
} // namespace network
