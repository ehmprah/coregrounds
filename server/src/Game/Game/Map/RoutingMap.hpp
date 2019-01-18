#pragma once

namespace game::map
{
	class RoutingMap :
		sl2::NonCopyable
	{
	private:
		using TileMap = ::game::map::TileMap;
		const TileMap& m_TileMap;
		sl2::container::Vector2D<int> m_Map;

		void _clear();

	public:
		RoutingMap(const TileMap& _tileMap);

		void generate(const TilePosition& start);
		std::vector<TilePosition> generatePath(const TilePosition& _destination) const;
		int get(const TilePosition& _pos) const;
		decltype(auto) getSize() const
		{
			return m_Map.getSize();
		}

		decltype(auto) getWidth() const
		{
			return m_Map.getWidth();
		}

		decltype(auto) getHeight() const
		{
			return m_Map.getHeight();
		}
	};

	bool isInRange(const RoutingMap& _map, const TilePosition& _pos);
} // namespace game::map
