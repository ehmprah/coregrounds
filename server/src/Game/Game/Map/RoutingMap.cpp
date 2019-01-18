#include "stdafx.h"
#include "RoutingMap.hpp"
#include "TileMap.hpp"
#include "Game/Entity/Unit.hpp"

namespace game::map
{
	/*#####
	# flood fill stuff
	#####*/
	class Node :
		sl2::NonCopyable
	{
	public:
		int value = 0;
		TilePosition position;

		Node(TilePosition _pos, int _value) :
			value(_value),
			position(std::move(_pos))
		{}

		Node(Node&& _other)
		{
			*this = std::move(_other);
		}

		Node& operator =(Node&& _other)
		{
			value = _other.value;
			position = std::move(_other.position);
			return *this;
		}
	};

	class Stack :
		sl2::NonCopyable
	{
	private:
		std::vector<Node> m_Stack;
		sl2::container::Bitset2D m_Check;

	public:
		Stack(const TileMap& _tileMap) :
			m_Check(_tileMap.getSize())
		{
			m_Stack.reserve(20);
		}

		void add(Node _node)
		{
			// insert nodes in descending order (lowest at least)
			if (!m_Check.isSet(_node.position))
			{
				m_Check.set(_node.position);
				m_Stack.insert(std::lower_bound(std::rbegin(m_Stack), std::rend(m_Stack), _node.value, [](const Node& _node, int _value){
					return _node.value < _value;
				}).base(), std::move(_node));
			}
		}

		Node takeNext()
		{
			assert(!m_Stack.empty());
			auto node = std::move(m_Stack.back());
			m_Stack.pop_back();
			return std::move(node);
		}

		bool isEmpty() const
		{
			return m_Stack.empty();
		}
	};

	/*#####
	# RoutingMap
	#####*/
	RoutingMap::RoutingMap(const TileMap& _tileMap) :
		m_TileMap(_tileMap),
		m_Map(_tileMap.getSize())
	{
		_clear();
		assert(m_Map.getCellCount() > 0);
	}

	void RoutingMap::_clear()
	{
		std::memset(m_Map.getRawData(), -1, m_Map.getCellCount()*sizeof(m_Map.at(0, 0)));
	}

	void RoutingMap::generate(const TilePosition& start)
	{
		_clear();

		Node startNode(start, 0);
		Stack stack(m_TileMap);
		stack.add(std::move(startNode));
		while (!stack.isEmpty())
		{
			auto node = stack.takeNext();
			m_Map.at(node.position) = node.value;
			// add neighbors
			for (int i = 0; i < 4; ++i)
			{
				auto x = node.position.getX();
				auto y = node.position.getY();
				switch (i)
				{
				case 0:
					if (x <= 0)
						continue;
					--x;
					break;
				case 1:
					if (y <= 0)
						continue;
					--y;
					break;
				case 2:
					if (x >= m_Map.getWidth() - 1)
						continue;
					++x;
					break;
				case 3:
					if (y >= m_Map.getHeight() - 1)
						continue;
					++y;
					break;
				}
				TilePosition pos(x, y);
				auto unit = m_TileMap.get(pos);
				if (!unit || unit->hasUnitFlag(unit::Flags::unsolid))
					stack.add(Node(pos, node.value + 1));
			}
		}
	}

	std::vector<TilePosition> RoutingMap::generatePath(const TilePosition& _destination) const
	{
		std::vector<TilePosition> result;
		auto pos = _destination;
		auto value = m_Map[pos];
		while (value > 0)
		{
			for (int i = 0; i < 4; ++i)
			{
				auto x = pos.getX();
				auto y = pos.getY();
				switch (i)
				{
				case 0:
					if (x <= 0)
						continue;
					--x;
					break;
				case 1:
					if (y <= 0)
						continue;
					--y;
					break;
				case 2:
					if (x >= m_Map.getWidth() - 1)
						continue;
					++x;
					break;
				case 3:
					if (y >= m_Map.getHeight() - 1)
						continue;
					++y;
					break;
				}
				TilePosition newPos(x, y);
				auto lookupValue = m_Map[newPos];
				if (value == lookupValue + 1)
				{
					result.push_back(newPos);
					pos = newPos;
					value = lookupValue;
					break;
				}
			}
		}
		return result;
	}

	int RoutingMap::get(const TilePosition& _pos) const
	{
		assert(isInRange(*this, _pos));
		return m_Map[_pos];
	}

	bool isInRange(const RoutingMap& _map, const TilePosition& _pos)
	{
		return 0 <= _pos.getX() && 0 <= _pos.getY() &&
			_pos.getX() < _map.getWidth() && _pos.getY() < _map.getHeight();
	}
} // namespace game::map
