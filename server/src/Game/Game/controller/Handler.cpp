#include "stdafx.h"
#include "Handler.hpp"
#include "PlayerController.hpp"

namespace game {
namespace controller {

	Handler::Handler(ParticipantData _participantData) :
		m_ParticipantData(std::move(_participantData))
	{
	}

	void Handler::handleEvent(protobuf::in::Message _msg)
	{
		std::scoped_lock<std::mutex> lock(m_ControllerMx);
		if (m_Controller)
			m_Controller->handleInput(std::move(_msg));
	}

	void Handler::setController(Player& _controller)
	{
		std::scoped_lock<std::mutex> lock(m_ControllerMx);
		m_Controller = &_controller;
	}

	const ParticipantData& Handler::getParticipantData() const
	{
		return m_ParticipantData;
	}

	void Handler::connected()
	{
		if (m_Controller)
			m_Controller->connected();
	}

	void Handler::disconnected()
	{
		if (m_Controller)
			m_Controller->disconnected();
	}
} // namespace controller
} // namespace game
