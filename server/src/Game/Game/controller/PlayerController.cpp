#include "stdafx.h"
#include "PlayerController.hpp"
#include "Handler.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"
#include "Game/Phase/PickPhase.hpp"
#include "Game/Phase/Serialize.hpp"
#include "Game/Phase/Patching.hpp"

namespace game::controller
{
	Player::Player(int _index, std::weak_ptr<Handler> _handler, InputHandler& _inputHandler, phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase) :
		super(_index, _pickPhase, _gamePhase, _finalizePhase),
		m_InputHandler(_inputHandler),
		m_Handler(_handler),
		m_ParticipantData(_handler.lock()->getParticipantData())
	{
	}

	template <class Phase>
	protobuf::out::Message _sendMessage(std::weak_ptr<Handler> _handler, const protobuf::out::Message& _old, int _participantIndex, bool _complete, const Phase& _phase)
	{
		if (auto ptr = _handler.lock())
		{
			auto msg = serializeToProtobuf(_phase, _participantIndex);
			ptr->sendMessage(_complete ? msg : createPatch(_old, msg));
			return msg;
		}
		return protobuf::out::Message();
	}

	void Player::_sendGamePhase()
	{
		m_Old = _sendMessage(m_Handler, m_Old, getParticipantIndex(), m_CompleteSerialization.exchange(false), m_GamePhase);
	}

	void Player::_sendPickPhase()
	{
		m_Old = _sendMessage(m_Handler, m_Old, getParticipantIndex(), m_CompleteSerialization.exchange(false), m_PickPhase);
	}

	void Player::_sendFinalizePhase()
	{
		m_Old = _sendMessage(m_Handler, m_Old, getParticipantIndex(), m_CompleteSerialization.exchange(false), m_FinalizePhase);
	}

	const ParticipantData& Player::getParticipantData()
	{
		return m_ParticipantData;
	}

	void Player::handleInput(protobuf::in::Message _msg)
	{
		m_InputHandler.add(getParticipantIndex(), std::move(_msg));
	}

	void Player::update(Time _diff)
	{
		Timer::State autoSurrenderTimerState = Timer::State::none;
		{
			std::scoped_lock<std::mutex> lock(m_AutoSurrenderMx);
			m_AutoSurrenderTimer.update(_diff);
			autoSurrenderTimerState = m_AutoSurrenderTimer.getState();
		}

		switch (getPhase())
		{
		case phase::Type::pick:
			_sendPickPhase();
			break;
		case phase::Type::game:
			_sendGamePhase();

			switch (autoSurrenderTimerState)
			{
			case Timer::State::finished:
			case Timer::State::justFinished:
			{
				auto& participant = m_GamePhase.getParticipants()[getParticipantIndex()];
				participant->surrender();
				break;
			}
			}
			break;
		case phase::Type::finalize:
			_sendFinalizePhase();
			break;
		}
	}

	void Player::setPhase(phase::Type _phase)
	{
		super::setPhase(_phase);
		m_CompleteSerialization = true;
	}

	void Player::connected()
	{
		m_CompleteSerialization = true;

		std::scoped_lock<std::mutex> lock(m_AutoSurrenderMx);
		m_AutoSurrenderTimer.reset();
	}

	void Player::disconnected()
	{
		std::scoped_lock<std::mutex> lock(m_AutoSurrenderMx);
		if (m_AutoSurrenderTimer.getState() == Timer::State::none)
			m_AutoSurrenderTimer.start(std::chrono::minutes(5));
	}
} // namespace game::controller
