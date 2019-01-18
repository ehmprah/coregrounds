#pragma once

namespace game::controller
{
	class InputHandler
	{
	public:
		InputHandler(phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase);

		void add(int _participantIndex, protobuf::in::Message _msg);
		void exec(phase::Type _curPhase);

	private:
		phase::Pick& m_PickPhase;
		phase::AbstractGame& m_GamePhase;
		phase::Finalize& m_FinalizePhase;

		std::mutex m_MessagesMx;
		std::vector<std::pair<int, protobuf::in::Message>> m_Messages;
	};
} // namespace game::controller
