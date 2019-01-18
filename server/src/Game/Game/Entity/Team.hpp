#pragma once

namespace game
{
	class Team :
		public sl2::operators::Equal<Team>
	{
	private:
		std::vector<int> m_Member;
		ID m_Id;

	public:
		Team(ID _id);

		ID getId() const;

		const std::vector<int>& getMemberIndexes() const;
		void addMember(int _playerIndex);

		friend bool operator ==(const Team& _lhs, const Team& _rhs);
	};
} // namespace game
