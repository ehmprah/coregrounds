#pragma once

#include "ControllerBase.hpp"
#include "Core/AI.hpp"

namespace game::controller
{
	class AI : public Base
	{
	private:
		using super = Base;

		ai::Ptr m_AI;
		InputHandler& m_InputHandler;

	public:
		AI(int _index, ai::Ptr _ai, InputHandler& _inputHandler, phase::Pick& _pickState, phase::AbstractGame& _gameState, phase::Finalize& _finalizePhase);

		const ParticipantData& getParticipantData() override;
		void update(Time _diff) override;
	};
} // namespace game::controller
