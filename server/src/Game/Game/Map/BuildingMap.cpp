#include "stdafx.h"
#include "BuildingMap.hpp"
#include "Game/Entity/Participant.hpp"
#include "Game/Entity/Wall.hpp"
#include "Map/TileMap.hpp"
#include "Map/PathMap.hpp"
#include "Map/ColliderMap.hpp"

namespace game::map
{
	BuildingMap::BuildingMap(const Participant& _participant) :
		m_Participant(_participant),
		m_FinalBuildingMap(_participant.getGameDependencies().tileMap.getSize()),
		m_StaticMap(_participant.getGameDependencies().tileMap.getSize())
	{
	}

	void BuildingMap::renewStaticMap(std::optional<TileRect> _region)
	{
		auto& pathMap = m_Participant.getGameDependencies().pathMap;
		auto& tileMap = m_Participant.getGameDependencies().tileMap;
		sl2::container::Bitset2D check(m_StaticMap.getSize());
		m_StaticMap = sl2::container::Bitset2D(m_StaticMap.getSize());
		auto forEachValidTile = [this, &check, &tileMap, &pathMap, _region](const TilePosition& _where) {
			for (auto y = -2; y <= 2; ++y)
			{
				for (auto x = -2; x <= 2; ++x)
				{
					if (!(std::abs(x) == 2 && std::abs(x) == std::abs(y)))		// do not track the outermost diagonal tiles
					{
						TilePosition pos(_where.getX() + x, _where.getY() + y);
						if (!m_StaticMap.isOutOfRange(pos) && !check[pos] && (!_region || sl2::geometry2d::contains(*_region, pos)))
						{
							m_StaticMap[pos] = !tileMap.get(pos) && !pathMap.isBlockingPosition(pos);
							check[pos] = true;
						}
					}
				}
			}
		};

		forEachValidTile(mapToTilePosition(m_Participant.getCorePosition()));
		for (auto wall : m_Participant.getWalls())
			forEachValidTile(mapToTilePosition(wall->getPosition()));

		renew(std::move(_region));
	}

	void BuildingMap::renew(std::optional<TileRect> _region)
	{
		auto& tileMap = m_Participant.getGameDependencies().tileMap;
		m_PossiblePositions.clear();
		auto& colliderMap = m_Participant.getGameDependencies().colliderMap;
		auto yEnd = _region ? _region->getBottom() : tileMap.getSize().getY();
		auto xEnd = _region ? _region->getRight() : tileMap.getSize().getX();
		for (auto y = _region ? _region->getTop() : TileCoordinateType(0); y < yEnd; ++y)
		{
			for (auto x = _region ? _region->getLeft() : TileCoordinateType(0); x < xEnd; ++x)
			{
				TilePosition pos(x, y);
				auto& cellEntities = colliderMap.getCell(pos)->getEntities();
				m_FinalBuildingMap[pos] = m_StaticMap[pos] && std::none_of(std::begin(cellEntities), std::end(cellEntities),
					[](auto _entity)
					{
						assert(_entity);
						return !_entity->getOwner().hasUnitFlag(unit::Flags::unsolid);
					}
				);
				if (m_FinalBuildingMap[pos])
					m_PossiblePositions.push_back(pos);
			}
		}
	}

	const sl2::container::Bitset2D& BuildingMap::getBuildingMap() const
	{
		return m_FinalBuildingMap;
	}

	const std::vector<TilePosition>& BuildingMap::getPossibleBuildPositions() const
	{
		return m_PossiblePositions;
	}
} // namespace game::map
