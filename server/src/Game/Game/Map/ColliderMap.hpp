#pragma once

#include "GridMap.hpp"

namespace game::map
{
	class ColliderMap : public GridMap<CellBase>
	{
	private:
		using super = GridMap<CellBase>;

	public:
		using Size = typename super::Size;
		ColliderMap(const Size& _tileMapSize);
	};
} // namespace game::map
