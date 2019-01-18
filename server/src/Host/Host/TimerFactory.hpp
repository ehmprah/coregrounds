#pragma once

class TimerFactory
{
public:
	TimerFactory(boost::asio::io_context& _ioContext);

	template <class Callback>
	boost::asio::deadline_timer createTimer(const boost::asio::deadline_timer::duration_type& _expiryTime, Callback&& _callback)
	{
		boost::asio::deadline_timer timer(m_IOContext, _expiryTime);
		timer.async_wait(m_Strand.wrap(_callback));
		return timer;
	}

private:
	boost::asio::io_context& m_IOContext;
	boost::asio::io_service::strand m_Strand{ m_IOContext };
};
