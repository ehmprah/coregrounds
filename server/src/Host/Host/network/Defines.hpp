#pragma once

namespace network
{
	class Error : public std::runtime_error
	{
	public:
		Error(std::string_view _what);
	};

	using IOService = boost::asio::io_service;
	using tcp = boost::asio::ip::tcp;

	using WebSocket = boost::beast::websocket::stream<tcp::socket>;
} // namespace network
