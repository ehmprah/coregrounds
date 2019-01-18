#pragma once

#include "Data/Shape.hpp"

namespace game
{
	class VisualInfo
	{
	private:
		ID m_Id;
		AbsPosition m_Pos;
		data::Shape m_Area;

	public:
		VisualInfo(ID _id, AbsPosition _pos, const data::Shape& _area);

		ID getId() const;
		const AbsPosition& getPosition() const;
		const data::Shape& getArea() const;
	};

	class VisualCollector
	{
	public:
		void swap();
		void resize(int _participantCount);

		void addEmote(int _participantId, ID _emoteId);
		const std::optional<ID>& getEmote(int _participantId) const;

		void addVisual(int _participantId, VisualInfo _visual);
		const std::vector<VisualInfo>& getVisuals(int _participantId) const;

	private:
		int m_ParticipantCount = 0;
		std::vector<std::optional<ID>> m_Emotes;
		std::vector<std::optional<ID>> m_OldEmotes;
		std::vector<std::vector<VisualInfo>> m_VisualInfos;
		std::vector<std::vector<VisualInfo>> m_OldVisualInfos;
	};
} // namespace game
