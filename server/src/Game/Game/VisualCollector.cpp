#include "stdafx.h"
#include "VisualCollector.hpp"

namespace game
{
	bool _isValidIndex(int _participantCount, int _participantIndex)
	{
		assert(0 < _participantCount);
		return 0 <= _participantIndex && _participantIndex < _participantCount;
	}

	VisualInfo::VisualInfo(ID _id, AbsPosition _pos, const data::Shape& _area) :
		m_Id(_id),
		m_Pos(_pos),
		m_Area(_area)
	{
	}

	ID VisualInfo::getId() const
	{
		return m_Id;
	}

	const AbsPosition& VisualInfo::getPosition() const
	{
		return m_Pos;
	}

	const data::Shape& VisualInfo::getArea() const
	{
		return m_Area;
	}

	void VisualCollector::swap()
	{
		std::swap(m_OldEmotes, m_Emotes);
		m_Emotes.clear();
		m_Emotes.resize(m_ParticipantCount);

		std::swap(m_OldVisualInfos, m_VisualInfos);
		m_VisualInfos.clear();
		m_VisualInfos.resize(m_ParticipantCount);
	}

	void VisualCollector::resize(int _participantCount)
	{
		assert(0 < _participantCount);
		m_ParticipantCount = _participantCount;
		m_OldVisualInfos.resize(_participantCount);
		m_OldEmotes.resize(_participantCount);
		m_VisualInfos.resize(_participantCount);
		m_Emotes.resize(_participantCount);
	}

	void VisualCollector::addEmote(int _participantId, ID _emoteId)
	{
		assert(0 < _emoteId);
		assert(_isValidIndex(m_ParticipantCount, _participantId));
		m_Emotes[_participantId] = _emoteId;
	}

	const std::optional<ID>& VisualCollector::getEmote(int _participantId) const
	{
		assert(_isValidIndex(m_ParticipantCount, _participantId));
		return m_OldEmotes[_participantId];
	}

	void VisualCollector::addVisual(int _participantId, VisualInfo _visual)
	{
		assert(0 < _visual.getId());
		assert(_isValidIndex(m_ParticipantCount, _participantId));
		m_VisualInfos[_participantId].emplace_back(std::move(_visual));
	}

	const std::vector<VisualInfo>& VisualCollector::getVisuals(int _participantId) const
	{
		assert(_isValidIndex(m_ParticipantCount, _participantId));
		return m_OldVisualInfos[_participantId];
	}
} // namespace game
