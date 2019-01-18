#pragma once

#include "BaseAI.hpp"
#include "Core/ParticipantData.hpp"

namespace game::ai
{
	class EmptyAI:
		public BaseAI
	{
	public:
		EmptyAI(const Initializer& _initializer) :
			BaseAI(_initializer),
			m_ParticipantData(generateParticipantData(_initializer.gameData, 0, { /*no mod pages*/}))
		{
		}

		const ParticipantData& getParticipantData() const override
		{
			return m_ParticipantData;
		}

		std::optional<protobuf::in::Match_Game> update(Time _diff, const protobuf::out::Match_Game& _msg) override
		{
			return std::nullopt;
		}

		std::optional<protobuf::in::Match_Pick> update(Time _diff, const protobuf::out::Match_Pick& _msg) override
		{
			return std::nullopt;
		}

		std::optional<protobuf::in::Match::Finalize> update(Time _diff, const protobuf::out::Match::Finalize& _msg) override
		{
			return std::nullopt;
		}

	private:
		ParticipantData m_ParticipantData;
	};
} // namespace game::ai
