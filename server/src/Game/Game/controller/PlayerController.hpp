#pragma once

#include "ControllerBase.hpp"

namespace game::controller
{
	class Handler;
	class Player :
		public Base
	{
	public:
		Player(int _index, std::weak_ptr<Handler> _handler, InputHandler& _inputHandler, phase::Pick& _pickState, phase::AbstractGame& _gameState, phase::Finalize& _finalizePhase);
		virtual ~Player() = default;

		const ParticipantData& getParticipantData() override;

		void connected();
		void disconnected();

		void handleInput(protobuf::in::Message _msg);

		void update(Time _diff) override;
		void setPhase(phase::Type _phase) override;

	private:
		using super = Base;

		std::weak_ptr<Handler> m_Handler;
		mutable std::atomic_bool m_CompleteSerialization{ true };
		protobuf::out::Message m_Old;

		InputHandler& m_InputHandler;
		ParticipantData m_ParticipantData;

		std::mutex m_AutoSurrenderMx;
		Timer m_AutoSurrenderTimer;

		void _sendPickPhase();
		void _sendGamePhase();
		void _sendFinalizePhase();
	};
} // namespace game::controller
