#include "stdafx.h"
#include "VisibilityMap.hpp"
#include "Game/Entity/Unit.hpp"

namespace game::map
{
	bool VisibilityMap::_isVisibleForCell(const AbsShape& _shape, const TilePosition& _pos) const
	{
		auto& cell = m_Grid[_pos];
		switch (m_Grid[_pos].state)
		{
		case TileState::visible: return true;
		case TileState::partialVisible:
			for (auto& shape : cell.shapes)
			{
				if (_shape.overlaps(_shape))
					return true;
			}
		}
		return false;
	}

	void VisibilityMap::_clear()
	{
		m_Grid.clear();
	}

	VisibilityMap::TileState VisibilityMap::getTileState(const TilePosition& _pos) const
	{
		return m_Grid[_pos].state;
	}

	void VisibilityMap::renewDynamicMap(const std::vector<const unit::Unit*>& _units)
	{
		_clear();

		for (auto unit : _units)
		{
			assert(unit);
			auto visRange = unit->calculateVisibilityRange();
			if (visRange > 0)
			{
				AbsCircle visArea(unit->getPosition().getX(), unit->getPosition().getY(), visRange);
				m_Grid.forEachCell(visArea, [&visArea](const TilePosition& _pos, Cell& _cell) {
					_cell.shapes.emplace_back(visArea);
					switch (_cell.state)
					{
					case TileState::visible: break;
					default:
						_cell.state = TileState::visible;
						break;
					}
				});
			}
		}

		// Add partials
		// TODO review and feedback please!
		int MapWidth = 15;
		int MapHeight = 7;
		for (auto y = 0; y < MapHeight; ++y)
		{
			for (auto x = 0; x < MapWidth; ++x)
			{
				TilePosition pos(x, y);
				auto& cell = m_Grid[pos];
				if (cell.state == TileState::visible)
				{
					for (int xOffset = -1; xOffset <= 1; xOffset++) {
						for (int yOffset = -1; yOffset <= 1; yOffset++) {
							int xMap = x + xOffset;
							int yMap = y + yOffset;
							if (0 <= xMap && xMap < MapWidth && 0 <= yMap && yMap < MapHeight) {
								TilePosition adjacentTile(xMap, yMap);
								auto& adjacentCell = m_Grid[adjacentTile];
								if (adjacentCell.state == TileState::hidden) adjacentCell.state = TileState::partialVisible;
							}
						}
					}
				}
			}
		}
	}

	bool VisibilityMap::isVisible(const unit::Unit& _unit) const
	{
		switch (_unit.getType())
		{
		case unit::Type::core: return true;
		case unit::Type::tower: return getTileState(mapToTilePosition(_unit.getPosition())) == map::VisibilityMap::TileState::visible;
		case unit::Type::wall: return getTileState(mapToTilePosition(_unit.getPosition())) != map::VisibilityMap::TileState::hidden;
		case unit::Type::minion:
		case unit::Type::projectile:
		case unit::Type::trigger:
			auto rect = _unit.getCollider().createBoundingRect();
			auto topLeft = mapToTilePosition(rect.getTopLeft());
			auto bottomRight = mapToTilePosition(rect.getTopLeft() + *rect.getSize());
			if (_isVisibleForCell(_unit.getCollider(), topLeft) || (topLeft != bottomRight && _isVisibleForCell(_unit.getCollider(), bottomRight)))
				return true;
		}
		return false;
	}
} // namespace game::map
