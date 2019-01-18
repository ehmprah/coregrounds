#pragma once

namespace game::unit
{
	class LifeLinkContainer :
		sl2::NonCopyable
	{
	private:
		std::vector<Unit*> m_Members;

		void _killAllUnits(const CastableSnapshot& _dealer, const Unit& _originalReceiver);

	public:
		const std::vector<Unit*> getMembers() const;

		void addMember(Unit& _unit);
		void removeMember(Unit& _unit);
	};

	class LifeLinkPool
	{
	private:
		std::vector<Unit*> m_Members;
		Stat m_PoolHP = 0;

	public:
		void insert(const LifeLinkContainer& _container);
		Stat getTotalHP() const;	// be careful: HP is calculated and cached when members are added
		std::vector<std::pair<Unit*, Stat>> calculateSharedDamage(Stat _damage) const;

		auto isEmpty() const
		{
			return m_Members.empty();
		}

		auto begin()
		{
			return m_Members.begin();
		}

		auto begin() const
		{
			return m_Members.begin();
		}

		auto end()
		{
			return m_Members.end();
		}

		auto end() const
		{
			return m_Members.end();
		}
	};
} // namespace game::unit
