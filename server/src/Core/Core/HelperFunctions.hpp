#pragma once

#include "Core/SharedDefines.hpp"

namespace game
{
	TilePosition mapToTilePosition(const AbsPosition& _pos);
	AbsPosition mapToAbsPosition(const TilePosition& _pos);
	AbsPosition makeTileCenterPosition(const AbsPosition& _pos);
	bool isAxisAligned(AbsCoordType _value);
	bool isTileCenter(const AbsPosition& _pos);

	bool isAlmostEqual(const AbsPosition& _lhs, const AbsPosition& _rhs);

	template <class EnumType>
	int toInt(EnumType _value)
	{
		int val = static_cast<int>(_value);
		if (val <= 0 || val >= static_cast<int>(EnumType::max))
			return 0;
		return val;
	}

	template <class EnumType>
	EnumType fromInt(int _value)
	{
		if (_value <= 0 || _value >= static_cast<int>(EnumType::max))
			return EnumType::none;
		return static_cast<EnumType>(_value);
	}

	template <class T>
	T setHighestBit(T _value)
	{
		static_assert(std::is_integral<T>::value, "only allowed for integral types.");
		T high = 1 << (sizeof(_value) * 8 - 1);
		assert(!(_value & high));
		return _value | high;
	}
} // namespace game
