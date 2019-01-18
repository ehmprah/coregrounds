#pragma once

namespace game::map
{
	class BuildingMap :
		sl2::NonCopyable
	{
	private: 
		const Participant& m_Participant;
		sl2::container::Bitset2D m_StaticMap;
		sl2::container::Bitset2D m_FinalBuildingMap;
		std::vector<TilePosition> m_PossiblePositions;

	public:
		BuildingMap(const Participant& _participant);

		void renewStaticMap(std::optional<TileRect> _region);
		void renew(std::optional<TileRect> _region);

		// ToDo: remove this function
		const sl2::container::Bitset2D& getBuildingMap() const;
		const std::vector<TilePosition>& getPossibleBuildPositions() const;
	};
} // namespace game::map
