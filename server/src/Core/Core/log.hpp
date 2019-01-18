#pragma once

#include "BoostInclude.hpp"
#include "StdInclude.hpp"
#include "SharedDefines.hpp"

namespace std
{
	std::ostream& operator <<(std::ostream& _out, const std::chrono::nanoseconds& _duration);
	std::ostream& operator <<(std::ostream& _out, const std::chrono::microseconds& _duration);
	std::ostream& operator <<(std::ostream& _out, const std::chrono::milliseconds& _duration);
	std::ostream& operator <<(std::ostream& _out, const std::chrono::seconds& _duration);
	std::ostream& operator <<(std::ostream& _out, const std::chrono::minutes& _duration);
	std::ostream& operator <<(std::ostream& _out, const std::chrono::hours& _duration);

	template <class T>
	std::ostream& operator <<(std::ostream& _out, const game::Vector2D<T>& _vec)
	{
		_out << "x: " << _vec.getX() << " y: " << _vec.getY();
		return _out;
	}

	template <class T>
	std::ostream& operator <<(std::ostream& _out, const std::optional<T>& _opt)
	{
		if (_opt)
			_out << *_opt;
		else
			_out << "not set";
		return _out;
	}
} // namespace std

enum class LogSeverityLevel
{
	debug,
	info,
	warning,
	error,
	fatal
};
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", LogSeverityLevel);

std::ostream& operator <<(std::ostream& _out, LogSeverityLevel level);

using ChannelLogger = boost::log::sources::severity_channel_logger_mt<LogSeverityLevel, std::string>;

void initLoggingSystem();

#define LOG_CHANNEL(logger, level, msg) BOOST_LOG_SEV(logger, level) << msg

//#ifndef NDEBUG
	#define LOG_CHANNEL_DEBUG(logger, msg) LOG_CHANNEL(logger, LogSeverityLevel::debug, msg)
//#else
//	#define LOG_CHANNEL_DEBUG(logger, msg) ((void)0)
//#endif

#define LOG_CHANNEL_INFO(logger, msg) LOG_CHANNEL(logger, LogSeverityLevel::info, msg)
#define LOG_CHANNEL_WARN(logger, msg) LOG_CHANNEL(logger, LogSeverityLevel::warning, msg)
#define LOG_CHANNEL_ERR(logger, msg) LOG_CHANNEL(logger, LogSeverityLevel::error, msg)
#define LOG_CHANNEL_FATAL(logger, msg) LOG_CHANNEL(logger, LogSeverityLevel::fatal, msg)

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(DefaultLogger, ChannelLogger, (boost::log::keywords::channel = "server"));
inline ChannelLogger& defaultLogger{ DefaultLogger::get() };

#define LOG_DEBUG(msg) LOG_CHANNEL_DEBUG(defaultLogger, msg) 
#define LOG_INFO(msg) LOG_CHANNEL_INFO(defaultLogger, msg)
#define LOG_WARN(msg) LOG_CHANNEL_WARN(defaultLogger, msg)
#define LOG_ERR(msg) LOG_CHANNEL_ERR(defaultLogger, msg)
#define LOG_FATAL(msg) LOG_CHANNEL_FATAL(defaultLogger, msg)
