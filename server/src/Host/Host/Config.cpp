#include "stdafx.h"
#include "Config.hpp"

void Config::load(std::string_view _fileName)
{
	boost::property_tree::ptree tree;
	boost::property_tree::read_ini(_fileName.data(), tree);

	networkPort = tree.get("network.port", networkPort);
	networkThreads = tree.get("network.threads", networkThreads);

	ssl_certificate = tree.get("ssl.certificate", ssl_certificate);
	ssl_key = tree.get("ssl.key", ssl_key);
	homePort = tree.get("web.home_port", homePort);
	homeIp = tree.get("web.home_ip", homeIp);
	userInfoUrl = tree.get("web.user_info_url", userInfoUrl);
	gameStatisticsUrl = tree.get("web.game_statistics_url", gameStatisticsUrl);
	gameConfigUrl = tree.get("web.game_config_url", gameConfigUrl);

	prepareMatchesMaxDuration = decltype(prepareMatchesMaxDuration)(tree.get("match.max_prepare_duration", prepareMatchesMaxDuration.count()));
}
