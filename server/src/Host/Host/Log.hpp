#pragma once

namespace network
{
	BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(InLogger, ChannelLogger, (boost::log::keywords::channel = "network-in"));
	BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(OutLogger, ChannelLogger, (boost::log::keywords::channel = "network-out"));

	inline ChannelLogger& inLogger{ InLogger::get() };
	inline ChannelLogger& outLogger{ OutLogger::get() };
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(HostLogger, ChannelLogger, (boost::log::keywords::channel = "host"));

inline ChannelLogger& hostLogger{ HostLogger::get() };