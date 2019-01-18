#pragma once

#include "Game/Phase/PhaseInterface.hpp"
#include "Game/controller/InputHandler.hpp"

namespace game::controller
{
	class Base :
		sl2::NonCopyable
	{
	public:
		Base(int _index, phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase) :
			m_ParticipantIndex(_index),
			m_PickPhase(_pickPhase),
			m_GamePhase(_gamePhase),
			m_FinalizePhase(_finalizePhase)
		{
			assert(0 <= m_ParticipantIndex);
		}

		virtual const ParticipantData& getParticipantData() = 0;

		virtual ~Base() = default;

		int getParticipantIndex() const
		{
			assert(0 <= m_ParticipantIndex);
			return m_ParticipantIndex;
		}

		virtual void setPhase(phase::Type _phase)
		{
			m_CurrentPhase = _phase;
		}

		phase::Type getPhase() const
		{
			return m_CurrentPhase;
		}

		virtual void update(Time _diff) = 0;

	protected:
		phase::Pick& m_PickPhase;
		phase::AbstractGame& m_GamePhase;
		phase::Finalize& m_FinalizePhase;

	private:
		int m_ParticipantIndex = 0;
		phase::Type m_CurrentPhase = phase::Type::none;
	};
	using ControllerPtr = std::unique_ptr<Base>;
} // namespace game::controller
