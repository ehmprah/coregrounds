#pragma once

#include "Defines.hpp"

namespace network
{
	class Connection :
		public std::enable_shared_from_this<Connection>,
		sl2::NonCopyable
	{
	public:
		Connection(WebSocket _socket);
		~Connection();

		bool isOpen() const;
		void close();
		void exec();

		std::future<bool> asyncWrite(std::string _data, bool _binary = false);
		using ReadBuffer = boost::beast::flat_buffer;

		tcp::endpoint getRemoteEndpoint() const;

		// signals
		using ReadSignal = boost::signals2::signal<void(const ReadBuffer&, bool/*true = binary*/)>;
		/*! \brief	read callback
		/* \details it is guaranteed that the callback will only emit once a time. */
		ReadSignal onRead;
		using DisconnectSignal = boost::signals2::signal<void()>;
		DisconnectSignal onDisconnect;

	private:
		struct WriteTask
		{
		public:
			WriteTask(std::string _buffer, bool _binary);

			std::promise<bool> promise;
			std::string buffer;
			bool binary = false;
			int executions = 0;
		};

		void _execWriteTask(WriteTask& _task);
		void _execAsyncWrite();
		void _onAsyncWrite(boost::system::error_code _ec, std::size_t _bytesTransferred);
		void _execAsyncRead();
		void _onAsyncRead(boost::system::error_code _ec, std::size_t _bytesTransferred);

		std::mutex m_ReadMutex;
		std::mutex m_WriteBuffersMutex;
		std::optional<WriteTask> m_CurWriteTask;
		std::deque<WriteTask> m_WriteTasks;
		ReadBuffer m_AsyncReadBuffer;
		WebSocket m_Socket;
		boost::asio::strand<boost::asio::io_context::executor_type> m_Strand;
	};
	using ConnectionPtr = std::shared_ptr<Connection>;
} // namespace network
