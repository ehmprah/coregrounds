#include "stdafx.h"
#include "HeatMap.hpp"

namespace game::ai 
{
	
	TilePosition HeatMap::getThreatPoint() const
	{
		auto& map = getHeatmap(HeatmapType::Damage);
		// Tile coordinates of the threat point
		int x = 0;
		int y = 6;
		// Loop over all points returning the hottest tile
		for (int mx = 0; mx < MapWidth; mx++)
		{
			for (int my = 0; my < MapHeight; my++)
			{
				if (0 < map[{mx, my}] && (mx > x || my < y))
				{
					// We're looking for the top-most, right-most point with heat
					x = mx;
					y = my;
				}
			}
		}
		return TilePosition(x, y);
	}

	std::optional<TilePosition> HeatMap::getHotTile(HeatmapType _type, int _minHeat) const
	{
		auto& map = getHeatmap(_type);
		int hottest = 0;
		TilePosition pos;
		for (int mx = 0; mx < MapWidth; ++mx) 
		{
			for (int my = 0; my < MapHeight; ++my) 
			{
				auto heat = map[{mx, my}];
				if (hottest < heat)
				{
					hottest = heat;
					pos.setX(mx);
					pos.setY(my);
				}
			}
		}
		if (_minHeat < hottest)
			return pos;
		return std::nullopt;
	}

	std::optional<AbsPosition> HeatMap::getHotSpot(HeatmapType _type, int _minHeat) const
	{
		auto& map = getHeatmap(_type);
		if (auto hotTile = getHotTile(_type, _minHeat))
			return AbsPosition(hotTile->getX() + 0.5, hotTile->getY() + 0.5);
		return std::nullopt;
	}

	int HeatMap::heatAtTile(HeatmapType _type, game::TilePosition _pos) const
	{
		auto& map = getHeatmap(_type);
		return map[{_pos.getX(), _pos.getY()}];
	}

	int HeatMap::heatAroundTile(HeatmapType _type, game::TilePosition _pos, int _range) const
	{
		auto& map = getHeatmap(_type);
		int heat = 0;
		int x = _pos.getX();
		int y = _pos.getY();
		for (int xOffset = -_range; xOffset <= _range; ++xOffset)
		{
			for (int yOffset = -_range; yOffset <= _range; ++yOffset)
			{
				int xMap = x + xOffset;
				int yMap = y + yOffset;
				if (!map.isOutOfRange({ xMap, yMap }))
					heat += map[{xMap, yMap}];
			}
		}
		return heat;
	}

	const sl2::container::Vector2D<int>& HeatMap::getHeatmap(HeatmapType _type) const
	{
		return m_Maps[static_cast<std::size_t>(_type)];
	}

	void HeatMap::buildDamageMap(const protobuf::out::Match_Game& _output)
	{
		sl2::container::Vector2D<int> map(MapWidth, MapHeight);
			
		auto playerIndex = 1 - removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);

		// Add towers to the heatmap
		for (auto& tower : player.towers()) 
			addHeat(map, deserializeTilePos(tower.unit().position()), true);

		// Add minions to the heatmap
		for (auto& minion : player.minions()) 
			addHeat(map, deserializeTilePos(minion.unit().position()), false);

		m_Maps[static_cast<std::size_t>(HeatmapType::Damage)] = std::move(map);
	}

	void HeatMap::buildHealingMap(const protobuf::out::Match_Game& _output)
	{
		sl2::container::Vector2D<int> map(MapWidth, MapHeight);

		auto playerIndex = removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);

		// Add towers to the heatmap
		for (auto& tower : player.towers())
		{
			if (removeDirtyFlag(tower.unit().hp()) < 50)
				addHeat(map, deserializeTilePos(tower.unit().position()), true);
		}

		// Add minions to the heatmap
		for (auto& minion : player.minions()) 
		{
			if (removeDirtyFlag(minion.unit().hp()) < 50)
				addHeat(map, deserializeTilePos(minion.unit().position()), false);
		}

		m_Maps[static_cast<std::size_t>(HeatmapType::Healing)] = std::move(map);
	}

	void HeatMap::update(const protobuf::out::Match_Game& _output)
	{
		buildDamageMap(_output);
		buildHealingMap(_output);
	}

	void addHeat(sl2::container::Vector2D<int>& _map, const TilePosition& _position, bool _extra)
	{
		for (int xOffset = -1; xOffset < 2; ++xOffset)
		{
			for (int yOffset = -1; yOffset < 2; ++yOffset)
			{
				int xMap = _position.getX() + xOffset;
				int yMap = _position.getY() + yOffset;
				if (!_map.isOutOfRange({ xMap, yMap }))
				{
					auto& tile = _map[{xMap, yMap}];
					// 121
					// 232
					// 121
					if (_extra && xOffset == 0) ++tile;
					if (_extra && yOffset == 0) ++tile;
					++tile;
				}
			}
		}
	}
}