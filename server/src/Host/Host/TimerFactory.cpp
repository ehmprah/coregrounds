#include "stdafx.h"
#include "TimerFactory.hpp"

TimerFactory::TimerFactory(boost::asio::io_context& _ioContext) :
	m_IOContext(_ioContext)
{
}
