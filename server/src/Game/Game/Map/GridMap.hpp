#pragma once

#include "Core/HelperFunctions.hpp"

namespace game::map
{
	class GridEntity;
	class IGridMap :
		sl2::NonCopyable
	{
		friend class GridEntity;
	protected:
		virtual void adjust(const GridEntity& _entity) = 0;
	};

	class GridEntity :
		sl2::NonCopyable
	{
	private:
		using Unit = unit::Unit;

		const AbsShape& m_Area;
		IGridMap& m_GridMap;
		Unit& m_Owner;

	public:
		GridEntity(const AbsShape& _area, Unit& _owner, IGridMap& _gridMap) :
			m_Area(_area),
			m_GridMap(_gridMap),
			m_Owner(_owner)
		{}

		const AbsShape& getShape() const
		{
			return m_Area;
		}

		void adjust()
		{
			m_GridMap.adjust(*this);
		}

		Unit& getOwner() const
		{
			return m_Owner;
		}
	};

	class CellBase :
		sl2::NonCopyable
	{
	private:
		AbsRect m_Area;
		std::vector<const GridEntity*> m_Entities;

	public:
		virtual void add(const GridEntity& _entity)
		{
			assert(std::find(std::begin(m_Entities), std::end(m_Entities), &_entity) == std::end(m_Entities));
			m_Entities.push_back(&_entity);
		}

		virtual void remove(const GridEntity& _entity)
		{
			auto itr = std::find(std::begin(m_Entities), std::end(m_Entities), &_entity);
			assert(itr != std::end(m_Entities));
			m_Entities.erase(itr);
		}

		const std::vector<const GridEntity*>& getEntities() const
		{
			return m_Entities;
		}

		bool hasObjects() const
		{
			return !m_Entities.empty();
		}

		void setArea(AbsRect _area)
		{
			m_Area = std::move(_area);
		}

		const AbsRect& getShape() const
		{
			return m_Area;
		}
	};

	template <class Cell = CellBase>
	class GridMap :
		public IGridMap
	{
		static_assert(std::is_base_of<CellBase, Cell>::value, "GridMap: Cell must be derived from CellBase.");

	private:
		using Map = sl2::container::Vector2D<Cell>;
		Map m_Map;

		using Unit = unit::Unit;

		using EntityPair = std::pair<std::unique_ptr<GridEntity>, std::vector<Cell*>>;
		std::vector<EntityPair> m_Entities;

		std::vector<Cell*> _calculateCells(const AbsRect& _area)
		{
			auto begin = mapToTilePosition(AbsPosition(std::max<AbsCoordType>(0, _area.getLeft()), std::max<AbsCoordType>(0, _area.getTop())));
			auto end = mapToTilePosition(AbsPosition(std::min<AbsCoordType>((m_Map.getWidth() - 1) * TileAbsoluteSize, _area.getRight()),
				std::min<AbsCoordType>((m_Map.getHeight() - 1) * TileAbsoluteSize, _area.getBottom())));

			std::vector<Cell*> result;
			for (auto y = begin.getY(); y <= end.getY(); ++y)
			{
				for (auto x = begin.getX(); x <= end.getX(); ++x)
					result.push_back(&m_Map.at(TilePosition(x, y)));
			}
			return result;
		}

		template <class TShape, typename = std::enable_if_t<sl2::geometry2d::IsShape<TShape>::value>>
		std::vector<Cell*> _calculateCells(const TShape& _shape)
		{
			auto bounding = sl2::geometry2d::generateBoundingRect(_shape);
			auto begin = mapToTilePosition(AbsPosition(std::max<AbsCoordType>(0, bounding.getLeft()), std::max<AbsCoordType>(0, bounding.getTop())));
			auto end = mapToTilePosition(AbsPosition(std::min<AbsCoordType>((m_Map.getWidth() - 1) * TileAbsoluteSize, bounding.getRight()),
				std::min<AbsCoordType>((m_Map.getHeight() - 1) * TileAbsoluteSize, bounding.getBottom())));

			std::vector<Cell*> result;
			for (auto y = begin.getY(); y <= end.getY(); ++y)
			{
				for (auto x = begin.getX(); x <= end.getX(); ++x)
				{
					auto& tile = m_Map.at(TilePosition(x, y));
					if (sl2::geometry2d::overlaps(_shape, tile.getShape()))
						result.push_back(&tile);
				}
			}
			return result;
		}

		std::vector<Cell*> _calculateCells(const AbsShape& _area)
		{
			auto bounding = _area.createBoundingRect();
			auto begin = mapToTilePosition(AbsPosition(std::max<AbsCoordType>(0, bounding.getLeft()), std::max<AbsCoordType>(0, bounding.getTop())));
			auto end = mapToTilePosition(AbsPosition(std::min<AbsCoordType>((m_Map.getWidth() - 1) * TileAbsoluteSize, bounding.getRight()),
				std::min<AbsCoordType>((m_Map.getHeight() - 1) * TileAbsoluteSize, bounding.getBottom())));

			std::vector<Cell*> result;
			for (auto y = begin.getY(); y <= end.getY(); ++y)
			{
				for (auto x = begin.getX(); x <= end.getX(); ++x)
				{
					auto& tile = m_Map.at(TilePosition(x, y));
					if (_area.overlaps(tile.getShape()))
						result.push_back(&tile);
				}
			}
			return result;
		}

		std::vector<Cell*> _calculateCells(const AbsLine& _line)
		{
			// create bounding rect
			assert(_line.getType() == sl2::geometry2d::LineType::segment);
			auto endPoint = sl2::geometry2d::calculateSecondPoint(_line);
			auto minMaxX = std::minmax(_line.getPosition().getX(), endPoint.getX());
			auto minMaxY = std::minmax(_line.getPosition().getY(), endPoint.getY());
			auto tmpCells = _calculateCells(AbsRect(minMaxX.first, minMaxY.first, minMaxX.second - minMaxX.first, minMaxY.second - minMaxY.first));
			std::vector<Cell*> cells;
			cells.reserve(tmpCells.size());
			// perform more accurate check
			for (auto cell : tmpCells)
			{
				assert(cell);
				if (sl2::geometry2d::intersects(_line, cell->getShape()) || sl2::geometry2d::contains(cell->getShape(), _line.getPosition()))
					cells.emplace_back(cell);
			}
			return cells;
		}

		auto _findEntity(const GridEntity* _entity)
		{
			assert(_entity != nullptr);
			return std::find_if(std::begin(m_Entities), std::end(m_Entities), [_entity](const EntityPair& _pair) {
				return _pair.first.get() == _entity;
			});
		}

		void adjust(const GridEntity& _entity)
		{
			auto itr = _findEntity(&_entity);
			assert(itr != std::end(m_Entities));

			assert(itr->first);
			for (auto cell : itr->second)
			{
				assert(cell);
				cell->remove(_entity);
			}

			auto cells = _calculateCells(_entity.getShape());
			for (auto cell : cells)
			{
				assert(cell);
				cell->add(_entity);
			}
			itr->second = std::move(cells);
		}

	public:
		using Size = typename Map::Size;
		GridMap(const Size& _mapSize) :
			m_Map(_mapSize)
		{
			assert(_mapSize.getX() > 0 && _mapSize.getY() > 0);
			AbsSize size(AbsPosition(TileAbsoluteSize, TileAbsoluteSize));
			for (auto y = 0; y < m_Map.getHeight(); ++y)
			{
				for (auto x = 0; x < m_Map.getWidth(); ++x)
				{
					TilePosition pos(x, y);
					m_Map[pos].setArea(AbsRect(mapToAbsPosition(pos), size));
				}
			}
		}

		const Map& getMap() const
		{
			return m_Map;
		}

		GridEntity& create(const AbsShape& _shape, Unit& _owner)
		{
			auto gridEntity = std::make_unique<GridEntity>(_shape, _owner, *this);
			auto cells = _calculateCells(_shape);
			auto& ref = *gridEntity;
			for (auto cell : cells)
			{
				assert(cell);
				cell->add(ref);
			}
			m_Entities.emplace_back(std::move(gridEntity), std::move(cells));
			return ref;
		}

		void destroy(const GridEntity& _entity)
		{
			auto itr = _findEntity(&_entity);
			assert(itr != std::end(m_Entities));
			for (auto cell : itr->second)
			{
				assert(cell);
				cell->remove(_entity);
			}
			m_Entities.erase(itr);
		}

		template <class TShape>
		std::vector<const Cell*> calculateCells(const TShape& _shape) const
		{
			return reinterpret_cast<std::vector<const Cell*>&>(const_cast<GridMap&>(*this)._calculateCells(_shape));
		}

		const Cell* getCell(const TilePosition& _pos) const
		{
			return &m_Map.at(_pos);
		}

		template <class TShape>
		std::vector<const GridEntity*> getUniqueFromCells(const TShape& _shape) const
		{
			std::vector<const GridEntity*> result;
			for (auto cell : calculateCells(_shape))
			{
				auto& entities = cell->getEntities();
				result.reserve(result.size() + entities.size());
				for (auto entity : entities)
				{
					auto itr = std::lower_bound(std::begin(result), std::end(result), entity);
					if (itr == std::end(result) || *itr != entity)
						result.insert(itr, entity);
				}
			}
			return result;
		}
	};
} // namespace game::map
