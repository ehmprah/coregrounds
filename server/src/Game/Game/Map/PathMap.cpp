#include "stdafx.h"
#include "PathMap.hpp"
#include "Game/Entity/Unit.hpp"

namespace game::map
{
	/*#####
	# AStar stuff
	#####*/
	class GraphModel
	{
	private:
		const TileMap& m_TileMap;
		std::optional<TilePosition> m_AdditionalPosition;

	public:
		GraphModel(const TileMap& _tileMap, std::optional<TilePosition> _additionalPosition = std::nullopt) :
			m_TileMap(_tileMap),
			m_AdditionalPosition(_additionalPosition)
		{}

		std::vector<TilePosition> getAdjacentVertices(const TilePosition& _v) const
		{
			std::vector<TilePosition> vertices;
			vertices.reserve(4);
			for (int i = 0; i < 4; ++i)
			{
				auto p = _v;
				switch (i)
				{
				case 0:	// left
					p.setX(p.getX() - 1);
					break;
				case 1:	// top
					p.setY(p.getY() - 1);
					break;
				case 2: // right
					p.setX(p.getX() + 1);
					break;
				case 3: // down
					p.setY(p.getY() + 1);
					break;
				}

				if (m_TileMap.isValidPosition(p) && (!m_AdditionalPosition || p != *m_AdditionalPosition))
				{
					auto unit = m_TileMap.get(p);
					if (!unit || unit->hasUnitFlag(unit::Flags::unsolid))
						vertices.emplace_back(p);
				}
			}
			return vertices;
		}
	};

	template <class VertexDescriptor, class Node>
	class NodeVector2d
	{
	private:
		using Grid = sl2::container::Vector2D<std::optional<Node>>;
		Grid m_Grid;
		using Size = typename Grid::Size;

		int m_Counter = 0;

	public:
		NodeVector2d(const Size& _size) :
			m_Grid(_size)
		{}

		const Node& at(const VertexDescriptor& _at) const
		{
			assert(m_Grid[_at]);
			return *m_Grid[_at];
		}

		template <class Compare, class TNode, typename = std::enable_if_t<std::is_convertible_v<TNode, Node>>>
		void insertOrReplaceIf(const VertexDescriptor& _v, TNode&& _node, Compare _comp)
		{
			auto& value = m_Grid[_v];
			if (!value)
			{
				value = std::move(_node);
				++m_Counter;
			}
			else if (_comp(_node, *value))
				value = std::move(_node);
		}

		template <class Compare>
		std::pair<VertexDescriptor, Node> takeIf(Compare _comp)
		{
			auto itr = std::min_element(std::begin(m_Grid), std::end(m_Grid), [_comp](const auto& _lhs, const auto& _rhs) {
				return _comp(*_lhs, *_rhs);
			});
			assert(itr != std::end(m_Grid));
			auto result = *itr;
			*itr = std::nullopt;
			return result;
		}

		bool isEmpty() const
		{
			return m_Counter == 0;
		}
	};

	template <class VertexDescriptor, class Node>
	class NodeVector
	{
	private:
		using Grid = sl2::container::Bitset2D;
		Grid m_Grid;
		using Size = typename Grid::Size;

		std::vector<std::pair<VertexDescriptor, Node>> m_Storage;

	public:
		NodeVector(const Size& _size) :
			m_Grid(_size)
		{}

		template <class Compare, class TNode, typename = std::enable_if_t<std::is_convertible_v<TNode, Node>>>
		void insertOrReplaceIf(const VertexDescriptor& _v, TNode&& _node, Compare _comp)
		{
			if (m_Grid[_v])
			{
				auto itr = std::find_if(std::begin(m_Storage), std::end(m_Storage), [&_v](const auto& _value) {
					return _v == _value.first;
				});
				assert(itr != std::end(m_Storage));
				if (_comp(_node, itr->second))
					itr->second = std::move(_node);
			}
			else
			{
				m_Storage.emplace_back(_v, std::move(_node));
				m_Grid[_v] = true;
			}
		}

		template <class Compare>
		std::pair<VertexDescriptor, Node> takeIf(Compare _comp)
		{
			auto itr = std::min_element(std::begin(m_Storage), std::end(m_Storage), [_comp](const auto& _lhs, const auto& _rhs) {
				return _comp(_lhs.second, _rhs.second);
			});
			assert(itr != std::end(m_Storage));
			auto result = *itr;
			std::swap(*itr, m_Storage.back());
			m_Storage.pop_back();
			return result;
		}

		bool isEmpty() const
		{
			return m_Storage.empty();
		}
	};

	auto distanceCalculator = [](const TilePosition& _lhs, const TilePosition& _rhs) {
		return 1;
	};

	auto _performeAStar(const TileMap& _tileMap, const TilePosition& _start, const TilePosition& _dest, std::optional<TilePosition> _block = std::nullopt)
	{
		auto heuristic = [&_dest](const auto& _pos) {
			return std::abs(_dest.getX() - _pos.getX()) + std::abs(_dest.getY() - _pos.getY());
		};
		using NodeProperties = sl2::graph::astar::NodeProperties<TilePosition, TileCoordinateType>;
		return sl2::graph::astar::search(GraphModel(_tileMap, _block), _start, _dest, heuristic, distanceCalculator,
			sl2::graph::visit::ContainerTracker<sl2::container::Bitset2D>(_tileMap.getSize()),
			NodeVector<TilePosition, NodeProperties>(_tileMap.getSize()), NodeVector2d<TilePosition, NodeProperties>(_tileMap.getSize()));
	}

	/*#####
	# PathMap
	#####*/
	PathMap::PathMap(const TileMap& _tileMap) :
		m_TileMap(_tileMap)
	{
	}

	void PathMap::setCorePositions(std::pair<TilePosition, TilePosition> _corePositions)
	{
		m_CorePositions = _corePositions;
		renew();
	}

	bool PathMap::_estimateIfBlockingPos(const TilePosition& _pos) const
	{
		assert(m_TileMap.isValidPosition(_pos));
		if (!m_PathMap[_pos])
			return false;
		// check neighbor tiles
		bool topBlock = false, bottomBlock = false, leftBlock = false, rightBlock = false;
		for (int i = 0; i < 8; ++i)
		{
			bool top = false, bottom = false, left = false, right = false;
			switch (i)
			{
			case 0:		// top left
				top = true;
				left = true;
				break;
			case 1:		// top
				top = true;
				break;
			case 2:		// top right
				top = true;
				right = true;
				break;
			case 3:		// left
				left = true;
				break;
			case 4:		// right
				right = true;
				break;
			case 5:		// bottom left
				bottom = true;
				left = true;
				break;
			case 6:		// bottom
				bottom = true;
				break;
			case 7:		// bottom right
				bottom = true;
				right = true;
				break;
			}
			assert(!(top && bottom) || !(left && right));

			auto curPos = _pos;
			if (left)
				curPos.setX(curPos.getX() - 1);
			else if (right)
				curPos.setX(curPos.getX() + 1);
			if (top)
				curPos.setY(curPos.getY() - 1);
			else if (bottom)
				curPos.setY(curPos.getY() + 1);

			if (m_TileMap.isValidPosition(curPos))
			{
				auto unit = m_TileMap.get(curPos);
				if (!unit || unit->hasUnitFlag(unit::Flags::unsolid))
					continue;
			}
			leftBlock |= left;
			rightBlock |= right;
			topBlock |= top;
			bottomBlock |= bottom;
			if (leftBlock && rightBlock || topBlock && bottomBlock)
				return true;
		}
		return false;
	}

	void PathMap::renew()
	{
		auto astar = _performeAStar(m_TileMap, m_CorePositions.first, m_CorePositions.second);
		assert(astar.foundDestination());
		m_PathMap = sl2::container::Bitset2D(m_TileMap.getSize());
		m_BlockingPosMap = sl2::container::Bitset2D(m_TileMap.getSize());
		m_Path = astar.extractPath();
		for (auto& at : m_Path)
		{
			m_PathMap[at] = true;
			if (_estimateIfBlockingPos(at) && !_performeAStar(m_TileMap, m_CorePositions.first, m_CorePositions.second, at).foundDestination())
				m_BlockingPosMap[at] = true;
		}
	}

	bool PathMap::isBlockingPosition(const TilePosition& _at) const
	{
		assert(!m_BlockingPosMap.isNull());
		return m_BlockingPosMap[_at];
	}
} // namespace game::map
