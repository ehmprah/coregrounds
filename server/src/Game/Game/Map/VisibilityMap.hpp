#pragma once

#include "GridMap.hpp"

namespace game::map
{
	class VisibilityMap
	{
	public:
		enum class TileState
		{
			hidden,
			partialVisible,
			visible
		};

	private:
		struct Cell
		{
			TileState state = TileState::hidden;
			std::vector<AbsCircle> shapes;
		};

		template <class T>
		class GridMap
		{
		private:
			sl2::container::Vector2D<T> m_Grid;

		public:
			using Size = typename sl2::container::Vector2D<T>::Size;
			GridMap(const Size& _size) :
				m_Grid(_size)
			{}

			void clear()
			{
				m_Grid = sl2::container::Vector2D<T>(m_Grid.getSize());
			}

			auto& getSize() const
			{
				return m_Grid.getSize();
			}

			const T& operator [](const TilePosition& _pos) const
			{
				return m_Grid[_pos];
			}

			T& operator [](const TilePosition& _pos)
			{
				return m_Grid[_pos];
			}

			void forEachCell(const AbsRect& _shape, std::function<void(const TilePosition& _pos, const T& _value)> _func) const
			{
				auto begin = mapToTilePosition(AbsPosition(std::max<AbsCoordType>(0, _shape.getLeft()), std::max<AbsCoordType>(0, _shape.getTop())));
				auto end = mapToTilePosition(AbsPosition(std::min<AbsCoordType>((m_Grid.getWidth() - 1) * TileAbsoluteSize, _shape.getRight()),
					std::min<AbsCoordType>((m_Grid.getHeight() - 1) * TileAbsoluteSize, _shape.getBottom())));
				for (auto y = begin.getY(); y <= end.getY(); ++y)
				{
					for (auto x = begin.getX(); x <= end.getX(); ++x)
					{
						TilePosition pos(x, y);
						_func(pos, m_Grid[pos]);
					}
				}
			}

			template <class TShape, typename = std::enable_if_t<sl2::geometry2d::IsShape<TShape>::value>>
			void forEachCell(const TShape& _shape, std::function<void(const TilePosition& _pos, const T& _value)> _func) const
			{
				auto bounding = sl2::geometry2d::generateBoundingRect(_shape);
				auto begin = mapToTilePosition(AbsPosition(std::max<AbsCoordType>(0, bounding.getLeft()), std::max<AbsCoordType>(0, bounding.getTop())));
				auto end = mapToTilePosition(AbsPosition(std::min<AbsCoordType>((m_Grid.getWidth() - 1) * TileAbsoluteSize, bounding.getRight()),
					std::min<AbsCoordType>((m_Grid.getHeight() - 1) * TileAbsoluteSize, bounding.getBottom())));
				for (auto y = begin.getY(); y <= end.getY(); ++y)
				{
					for (auto x = begin.getX(); x <= end.getX(); ++x)
					{
						TilePosition pos(x, y);
						auto absPos = mapToAbsPosition(pos);
						AbsRect cellRect(absPos.getX(), absPos.getY(), TileAbsoluteSize, TileAbsoluteSize);
						if (sl2::geometry2d::overlaps(_shape, cellRect))
							_func(pos, m_Grid[pos]);
					}
				}
			}

			template <class TShape>
			void forEachCell(const TShape& _shape, std::function<void(const TilePosition& _pos, T& _value)> _func)
			{
				static_cast<const GridMap&>(*this).forEachCell(_shape, [_func](const TilePosition& _pos, const T& _value) {
					_func(_pos, const_cast<T&>(_value));
				});
			}
		};

		GridMap<Cell> m_Grid;

		bool _isVisibleForCell(const AbsShape& _shape, const TilePosition& _pos) const;
		void _clear();

	public:
		using Size = typename sl2::container::Vector2D<TileState>::Size;
		VisibilityMap(const Size& _size) :
			m_Grid(_size)
		{}

		auto& getSize() const
		{
			return m_Grid.getSize();
		}

		TileState getTileState(const TilePosition& _pos) const;

		void renewDynamicMap(const std::vector<const unit::Unit*>& _units);
		bool isVisible(const unit::Unit& _unit) const;
	};
} // namespace game::map
