#include "HelperFunctions.hpp"

namespace game
{
	TilePosition mapToTilePosition(const AbsPosition& _pos)
	{
		assert(_pos.getX() >= 0 && _pos.getY() >= 0);
		return static_cast<TilePosition>(_pos);
	}

	AbsPosition mapToAbsPosition(const TilePosition& _pos)
	{
		assert(_pos.getX() <= std::numeric_limits<AbsCoordType>::max() && _pos.getX() >= 0 &&
			_pos.getY() <= std::numeric_limits<AbsCoordType>::max() && _pos.getY() >= 0);
		return static_cast<AbsPosition>(_pos);
	}

	AbsPosition makeTileCenterPosition(const AbsPosition& _pos)
	{
		return mapToAbsPosition(mapToTilePosition(_pos)) + TileAbsoluteSize / 2;
	}

	bool isAxisAligned(AbsCoordType _value)
	{
		return sl2::isAlmostEqual(_value - std::floor(_value), TileAbsoluteSize / 2);
	}

	bool isTileCenter(const AbsPosition& _pos)
	{
		return isAxisAligned(_pos.getX()) && isAxisAligned(_pos.getY());
	}

	bool isAlmostEqual(const AbsPosition& _lhs, const AbsPosition& _rhs)
	{
		return sl2::isAlmostEqual(_lhs.getX(), _rhs.getX()) &&
			sl2::isAlmostEqual(_lhs.getY(), _rhs.getY());
	}
} // namespace game
