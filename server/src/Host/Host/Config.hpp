#pragma once

class Config
{
public:
	uint16_t networkPort{ 1337 };
	std::string ssl_key{ "./ssl/server.key" };
	std::string ssl_certificate{ "./ssl/server.crt" };

	int networkThreads = 1;
	uint16_t homePort;
	std::string homeIp;
	std::string userInfoUrl;
	std::string gameStatisticsUrl;
	std::string gameConfigUrl;

	std::chrono::seconds prepareMatchesMaxDuration{ 10 };

public:
	static constexpr const char* DefaultFileName = "host.conf";

	void load(std::string_view _fileName = DefaultFileName);
};
