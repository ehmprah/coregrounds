#pragma once

#include "Game/controller/Handler.hpp"

namespace session
{
	class AbstractHandler :
		sl2::NonCopyable
	{
	public:
		virtual ~AbstractHandler() = default;

		using WriteSignal = boost::signals2::signal<void(const protobuf::out::Message&)>;
		WriteSignal onWrite;

		virtual void handleEvent(protobuf::in::Message _msg) = 0;
		virtual void disconnected() = 0;
		virtual void connected() = 0;
	};

	class PlayerControllerHandler :
		public AbstractHandler,
		public game::controller::Handler
	{
	public:
		PlayerControllerHandler(game::ParticipantData _participantData);
		virtual ~PlayerControllerHandler() = default;

		virtual void handleEvent(protobuf::in::Message _msg) override;
		virtual void sendMessage(const protobuf::out::Message& _msg) override;
		virtual void connected() override;
		virtual void disconnected() override;
	};
} // namespace session
