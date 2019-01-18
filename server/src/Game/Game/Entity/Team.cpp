#include "stdafx.h"
#include "Team.hpp"
#include "Participant.hpp"

namespace game
{
	Team::Team(ID _id) :
		m_Id(_id)
	{
		assert(m_Id > 0);
	}

	ID Team::getId() const
	{
		return m_Id;
	}

	const std::vector<int>& Team::getMemberIndexes() const
	{
		return m_Member;
	}

	void Team::addMember(int _playerIndex)
	{
		auto itr = std::lower_bound(std::begin(m_Member), std::end(m_Member), _playerIndex);
		assert(itr == std::end(m_Member));
		m_Member.emplace(itr, _playerIndex);
	}

	bool operator ==(const Team& _lhs, const Team& _rhs)
	{
		return _lhs.getId() == _rhs.getId();
	}
} // namespace game
