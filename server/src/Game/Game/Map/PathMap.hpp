#pragma once

#include "TileMap.hpp"

namespace game::map
{
	class PathMap :
		sl2::NonCopyable
	{
	private:
		const TileMap& m_TileMap;
		std::pair<TilePosition, TilePosition> m_CorePositions;

		std::vector<TilePosition> m_Path;
		sl2::container::Bitset2D m_PathMap;
		sl2::container::Bitset2D m_BlockingPosMap;

		bool _estimateIfBlockingPos(const TilePosition& _pos) const;

	public:
		PathMap(const TileMap& _tileMap/*, std::pair<TilePosition, TilePosition> _corePositions*/);

		void setCorePositions(std::pair<TilePosition, TilePosition> _corePositions);

		void renew();
		bool isBlockingPosition(const TilePosition& _at) const;
	};
} // namespace game::map
