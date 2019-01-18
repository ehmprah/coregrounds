#include "stdafx.h"
#include "TileMap.hpp"
#include "Game/Entity/Unit.hpp"
#include "Core/HelperFunctions.hpp"

namespace game::map
{
	TileMap::TileMap(const Size& _tileMapSize) :
		m_Field(_tileMapSize)
	{
		assert(!m_Field.isNull());
	}

	bool TileMap::isValidPosition(const TilePosition& _pos) const
	{
		return !m_Field.isOutOfRange(_pos);
	}

	const TileMap::Size& TileMap::getSize() const
	{
		return m_Field.getSize();
	}

	unit::Unit* TileMap::get(const TilePosition& _tile) const
	{
		return m_Field[_tile];
	}

	void TileMap::set(unit::Unit& _unit)
	{
		assert(m_Units.find(&_unit) == std::end(m_Units));
		auto tilePos = mapToTilePosition(_unit.getPosition());
		if (auto unit = m_Field[tilePos])
			throw std::runtime_error("TileMap x: " + std::to_string(tilePos.getX()) + " y: " + std::to_string(tilePos.getY()) + " is already occupied.");
		m_Field[tilePos] = &_unit;
		auto result = m_Units.emplace(&_unit);
		assert(result.second);
	}

	void TileMap::remove(unit::Unit& _unit)
	{
		auto tilePos = mapToTilePosition(_unit.getPosition());
		assert(m_Field.at(tilePos) == &_unit);
		if (auto itr = m_Units.find(&_unit);
			itr != std::end(m_Units))
		{
			m_Units.erase(itr);
			m_Field[tilePos] = nullptr;
		}
	}
} // namespace game::map
