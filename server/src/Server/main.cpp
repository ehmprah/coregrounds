#include "stdafx.h"
#include "Host/Host.hpp"
#include "Host/Log.hpp"

int main(int argc, char *argv[])
{
	boost::filesystem::path::imbue(std::locale("C"));
	try
	{
		initLoggingSystem();
		Config conf;
		conf.load();

		LOG_CHANNEL_INFO(hostLogger, "Begin starting host system on threadId: " << std::this_thread::get_id());
		Host host(conf);
		host.run();
	}
	catch (const std::exception& _e)
	{
		LOG_FATAL("main: " << _e.what());
	}
	catch (...)
	{
		LOG_FATAL("main: unknown error.");
	}

	boost::log::core::get()->remove_all_sinks();
}
