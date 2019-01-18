#include "stdafx.h"
#include "Handler.hpp"

namespace session
{
	PlayerControllerHandler::PlayerControllerHandler(game::ParticipantData _participantData) :
		game::controller::Handler(std::move(_participantData))
	{
	}

	void PlayerControllerHandler::handleEvent(protobuf::in::Message _msg)
	{
		if (_msg.has_ping())
		{
			protobuf::out::Message msg;
			msg.mutable_pong();
			sendMessage(msg);
		}
		else
			game::controller::Handler::handleEvent(_msg);
	}

	void PlayerControllerHandler::sendMessage(const protobuf::out::Message& _msg)
	{
		onWrite(_msg);
	}

	void PlayerControllerHandler::connected()
	{
		game::controller::Handler::connected();
	}

	void PlayerControllerHandler::disconnected()
	{
		game::controller::Handler::disconnected();
	}
} // namespace session
