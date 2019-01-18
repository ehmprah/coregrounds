#include "stdafx.h"
#include "Connection.hpp"

namespace network
{
	Connection::Connection(WebSocket _socket) :
		m_Socket{ std::move(_socket) },
		m_Strand{ m_Socket.get_executor() }
	{
		assert(m_Socket.is_open());
	}

	Connection::~Connection()
	{
		try
		{
			close();
		}
		catch (const std::exception& _e)
		{
			LOG_ERR("network::Connection: destructor: " << _e.what());
		}
		catch (...)
		{
			LOG_ERR("network::Connection: destructor: unknown exception.");
		}
	}

	bool Connection::isOpen() const
	{
		return m_Socket.next_layer().is_open();
	}

	void Connection::close()
	{
		onDisconnect();
		if (m_Socket.next_layer().is_open())
		{
			m_Socket.next_layer().shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
			m_Socket.next_layer().close();
		}
	}

	void Connection::exec()
	{
		_execAsyncRead();
	}

	std::future<bool> Connection::asyncWrite(std::string _data, bool _binary)
	{
		std::future<bool> future;
		{
			std::scoped_lock<std::mutex> lock(m_WriteBuffersMutex);
			m_WriteTasks.emplace_back(std::move(_data), _binary);
			future = m_WriteTasks.back().promise.get_future();
		}
		_execAsyncWrite();
		return future;
	}

	void Connection::_execWriteTask(WriteTask& _task)
	{
		if (!m_Socket.is_open())
			return;
		assert(0 <= _task.executions && _task.executions < std::numeric_limits<decltype(_task.executions)>::max());
		++_task.executions;

		_task.binary ? m_Socket.binary(true) : m_Socket.text(true);
		m_Socket.async_write(boost::asio::buffer(_task.buffer), boost::asio::bind_executor(m_Strand,
			[ptr = shared_from_this()](auto _ec, auto _bytesTransferred)
			{
				assert(ptr);
				ptr->_onAsyncWrite(_ec, _bytesTransferred);
			}
		));
	}

	void Connection::_execAsyncWrite()
	{
		std::scoped_lock<std::mutex> lock(m_WriteBuffersMutex);
		if (!m_CurWriteTask && !m_WriteTasks.empty())
		{
			m_CurWriteTask = std::move(m_WriteTasks.front());
			m_WriteTasks.pop_front();
			_execWriteTask(*m_CurWriteTask);
		}
	}

	void Connection::_onAsyncWrite(boost::system::error_code _ec, std::size_t _bytesTransferred)
	{
		assert(m_CurWriteTask);
		if (_ec)
		{
			if (m_CurWriteTask->executions <= 5)
			{
				LOG_ERR("network::Connection: ansyncWrite: " << _ec << " \"" << _ec.message() << "\" -> retry");
				std::scoped_lock<std::mutex> lock(m_WriteBuffersMutex);
				m_WriteTasks.emplace_front(std::move(*m_CurWriteTask));
				
			}
			else
			{
				m_CurWriteTask->promise.set_value(false);
				LOG_ERR("network::Connection: ansyncWrite: " << _ec << " \"" << _ec.message() << "\" -> skip");
			}
		}
		else
			m_CurWriteTask->promise.set_value(true);
		m_CurWriteTask.reset();
		_execAsyncWrite();
	}

	tcp::endpoint Connection::getRemoteEndpoint() const
	{
		return m_Socket.next_layer().remote_endpoint();
	}

	void Connection::_execAsyncRead()
	{
		if (m_Socket.is_open())
		{
			m_Socket.async_read(m_AsyncReadBuffer, boost::asio::bind_executor(m_Strand,
				[weakPtr = weak_from_this()](auto _ec, auto _bytesTransferred){
					if (auto ptr = weakPtr.lock())
						ptr->_onAsyncRead(_ec, _bytesTransferred);
				}
			));
		}
	}

	void Connection::_onAsyncRead(boost::system::error_code _ec, std::size_t _bytesTransferred)
	{
		std::scoped_lock<std::mutex> lock(m_ReadMutex);
		using error = boost::beast::websocket::error;
		//if (_ec == error::closed ||
		//	_ec.value() == 10054)	// remote closed
		if (_ec)
		{
			LOG_ERR("network::Connection: onAsyncRead: " << _ec << " \"" << _ec.message() << "\"");
			close();
			return;
		}

		if (_ec)
			LOG_ERR("network::Connection: onAsyncRead: " << _ec << " \"" << _ec.message() << "\"");
		else
			onRead(m_AsyncReadBuffer, m_Socket.got_binary());

		m_AsyncReadBuffer.consume(_bytesTransferred);
		_execAsyncRead();
	}

	Connection::WriteTask::WriteTask(std::string _buffer, bool _binary) :
		buffer(std::move(_buffer)),
		binary(_binary)
	{
	}
} // namespace network
