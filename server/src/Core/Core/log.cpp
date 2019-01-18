#include "log.hpp"

namespace std
{
	std::ostream& operator <<(std::ostream& _out, const std::chrono::nanoseconds& _duration)
	{
		_out << _duration.count() << " ns";
		return _out;
	}

	std::ostream& operator <<(std::ostream& _out, const std::chrono::microseconds& _duration)
	{
		_out << _duration.count() << " us";
		return _out;
	}

	std::ostream& operator <<(std::ostream& _out, const std::chrono::milliseconds& _duration)
	{
		_out << _duration.count() << " ms";
		return _out;
	}

	std::ostream& operator <<(std::ostream& _out, const std::chrono::seconds& _duration)
	{
		_out << _duration.count() << " s";
		return _out;
	}

	std::ostream& operator <<(std::ostream& _out, const std::chrono::minutes& _duration)
	{
		_out << _duration.count() << " m";
		return _out;
	}

	std::ostream& operator <<(std::ostream& _out, const std::chrono::hours& _duration)
	{
		_out << _duration.count() << " h";
		return _out;
	}
} // namespace std

std::ostream& operator <<(std::ostream& _out, LogSeverityLevel _lvl)
{
	constexpr const char* strings[] = { "debug", "info", "warning", "error", "fatal" };
	auto index = static_cast<std::size_t>(_lvl);
	if (index < std::size(strings))
		_out << strings[index];
	else
		_out << index;
	return _out;
}

void initLoggingSystem()
{
	boost::log::add_common_attributes();
	boost::log::register_simple_formatter_factory<LogSeverityLevel, char>("Severity");

	std::ifstream file("logger.conf");
	boost::log::init_from_stream(file);
}
