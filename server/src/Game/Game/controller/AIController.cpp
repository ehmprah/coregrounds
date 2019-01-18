#include "stdafx.h"
#include "AIController.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"
#include "Game/Phase/PickPhase.hpp"
#include "Game/Phase/Serialize.hpp"

namespace game::controller
{
	AI::AI(int _index, ai::Ptr _ai, InputHandler& _inputHandler, phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase) :
		super(_index, _pickPhase, _gamePhase, _finalizePhase),
		m_AI(std::move(_ai)),
		m_InputHandler(_inputHandler)
	{
		assert(m_AI);
	}

	const ParticipantData& AI::getParticipantData()
	{
		assert(m_AI);
		return m_AI->getParticipantData();
	}

	void AI::update(Time _diff)
	{
		assert(m_AI);
		switch (getPhase())
		{
		case phase::Type::pick:
			if (auto pickMsg = m_AI->update(_diff, serializeToProtobuf(m_PickPhase, getParticipantIndex()).match().pick()))
			{
				protobuf::in::Message msg;
				*msg.mutable_match()->mutable_pick() = std::move(*pickMsg);
				m_InputHandler.add(getParticipantIndex(), std::move(msg));
			}
			break;
		case phase::Type::game:
			if (auto gameMsg = m_AI->update(_diff, serializeToProtobuf(m_GamePhase, getParticipantIndex()).match().game()))
			{
				protobuf::in::Message msg;
				*msg.mutable_match()->mutable_game() = std::move(*gameMsg);
				m_InputHandler.add(getParticipantIndex(), std::move(msg));
			}
			break;
		case phase::Type::finalize:
			if (auto finalizeMsg = m_AI->update(_diff, serializeToProtobuf(m_FinalizePhase, getParticipantIndex()).match().finalize()))
			{
				protobuf::in::Message msg;
				*msg.mutable_match()->mutable_finalize() = std::move(*finalizeMsg);
				m_InputHandler.add(getParticipantIndex(), std::move(msg));
			}
			break;
		}
	}
} // namespace game::controller
