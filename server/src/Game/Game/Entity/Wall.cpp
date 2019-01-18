#include "stdafx.h"
#include "Wall.hpp"
#include "Participant.hpp"
#include "Data/Unit.hpp"
#include "Data/Game.hpp"
#include "Tower.hpp"
#include "Map/TileMap.hpp"
#include "Shape.hpp"

namespace game::unit
{
	Wall::Wall(const WallInitializer& _initializer) :
		Locatable(createShape(_initializer.wallData.getBody())),
		super(_initializer.unitInit),
		wallData(_initializer.wallData),
		m_MasterHandle(*this)
	{
		_setPosition(_initializer.pos);
	}

	WallHandle Wall::getHandle() const
	{
		return m_MasterHandle.get();
	}

	void Wall::addIntoWorld()
	{
		if (!isInWorld())
		{
			getGameDependencies().tileMap.set(*this);
			_setupChainBonus(_createChainInfo());
		}
		super::addIntoWorld();
	}

	void Wall::removeFromWorld()
	{
		if (isInWorld())
		{
			getGameDependencies().tileMap.remove(*this);
			_clearChainBonus();
		}
		super::removeFromWorld();
	}

	void Wall::calculateChainBonus()
	{
		_setupChainBonus(_createChainInfo());
	}

	Wall::ChainInfo Wall::_createChainInfo() const
	{
		auto& fieldSize = getGameDependencies().gameData.gameProperties.game.fieldSize;
		auto& tileMap = getGameDependencies().tileMap;
		sl2::container::Bitset2D check(*fieldSize);
		std::vector<TilePosition> openList{ mapToTilePosition(getPosition()) };
		check.set(static_cast<decltype(check)::Point>(openList.front()));

		std::pair<std::uint32_t, std::vector<Wall*>> result{ 0, {} };
		while (!std::empty(openList))
		{
			auto pos = openList.back();
			openList.pop_back();
			for (int i = 0; i < 4; ++i)
			{
				auto newPos = pos;
				switch (i)
				{
					// left
				case 0: newPos.setX(newPos.getX() - 1); break;
					// top
				case 1: newPos.setY(newPos.getY() - 1); break;
					// right
				case 2: newPos.setX(newPos.getX() + 1); break;
					// bottom
				case 3: newPos.setY(newPos.getY() + 1); break;
				}
				if (check.isOutOfRange(newPos) || check.isSet(newPos))
					continue;
				check.set(static_cast<decltype(check)::Point>(newPos));
				auto unit = tileMap.get(newPos);
				if (!unit || isHostile(*unit, *this))
					continue;

				switch (unit->getType())
				{
				case Type::wall:
					if (unit->isAlive())
					{
						result.second.emplace_back(&static_cast<Wall&>(*unit));
						++result.first;
						openList.emplace_back(newPos);
					}
					break;
				case Type::tower:
				{
					auto wall = static_cast<Tower&>(*unit).getWall();
					if (wall && wall->isAlive())
					{
						result.second.emplace_back(wall.get());
						++result.first;
						openList.emplace_back(newPos);
					}
					break;
				}
				case Type::core:
					++result.first;
					break;
				}
			}
		}
		return result;
	}

	void Wall::_setupChainBonus(ChainInfo _chainInfo)
	{
		for (auto wall : _chainInfo.second)
		{
			assert(wall);
			wall->m_ChainBonus = _chainInfo.first;
			LOG_DEBUG("Wall GUID: " << wall->getGUID() << " chain bonus: " << wall->m_ChainBonus);
		}
		m_ChainBonus = _chainInfo.first;
		LOG_DEBUG("Wall GUID: " << getGUID() << " chain bonus: " << m_ChainBonus);
	}

	void Wall::_clearChainBonus()
	{
		/* Before we call this method, we have to remove this wall from the tile map. 
		After that, we run the chaining algorithm, to find all other affected walls (the position from this wall is just our
		starting point, doesn't matter if something is here or not). Now we iterate through the affected walls and create chainInfos
		from their point of view. We remove all now affected walls from our previous list to keep the overhead as low as possible. */
		if (m_ChainBonus > 0)
		{
			auto chainInfo = _createChainInfo();
			auto& walls = chainInfo.second;
			while (!walls.empty())
			{
				auto wall = walls.back();
				walls.pop_back();
				auto pack = wall->_createChainInfo();
				for (auto packEl : pack.second)
				{
					auto itr = std::find(std::begin(walls), std::end(walls), packEl);
					assert(itr != std::end(walls));
					walls.erase(itr);
				}
				wall->_setupChainBonus(std::move(pack));
			}
			m_ChainBonus = 0;
		}
		LOG_DEBUG("Wall GUID: " << getGUID() << " cleared chain bonus");
	}

	void Wall::_setPosition(const TilePosition& _pos)
	{
		auto newPos = mapToAbsPosition(_pos) + TileAbsoluteSize / 2;
		if (newPos != getPosition())
		{
			auto& tileMap = getGameDependencies().tileMap;
			Locatable::setPosition(newPos);
			super::adjustCollider();
			if (isInWorld())
			{
				assert(tileMap.get(_pos) == nullptr);
				tileMap.set(*this);
				calculateChainBonus();
			}
		}
	}

	Stat Wall::_calculateChainArmorBonus() const
	{
		if (auto wallState = std::get_if<WallState>(&getUnitState().derivedState))
			return wallState->chainBonus * *getGameDependencies().gameData.gameProperties.game.wallChainArmorBonus;
		return 0;
	}

	void Wall::cleanUp()
	{
		super::cleanUp();
		_clearChainBonus();
	}

	bool Wall::hasNeighbour() const
	{
		const auto startPos = mapToTilePosition(getPosition());
		for (int i = 0; i < 8; ++i)
		{
			auto pos = startPos;
			switch (i)
			{
			case 0:				// top left
				if (pos.getX() == 0 || pos.getY() == 0)
					continue;
				pos -= 1;
				break;
			case 1:				// top
				if (pos.getY() == 0)
					continue;
				pos.setY(pos.getY() - 1);
				break;
			case 2:				// top right
				if (pos.getY() == 0)
					continue;
				pos.setY(pos.getY() - 1);
				pos.setX(pos.getX() + 1);
				break;
			case 3: pos.setX(pos.getX() + 1); break;	// right
			case 4: pos += 1; break;					// bottom right
			case 5: pos.setY(pos.getY() + 1); break;	// bottom
			case 6:				// bottom left
				if (pos.getX() == 0)
					continue;
				pos.setY(pos.getY() + 1);
				pos.setX(pos.getX() - 1);
				break;
			case 7:				// left
				if (pos.getX() == 0)
					continue;
				pos.setX(pos.getX() - 1);
				break;
			}
			if (getGameDependencies().tileMap.isValidPosition(pos))
			{
				auto wall = getGameDependencies().tileMap.get(pos);
				if (wall && &wall->getOwner() == &getOwner())
					return true;
			}
		}
		return false;
	}

	const AbsShape& Wall::getCollider() const
	{
		return Locatable::getArea();
	}

	AbsPosition Wall::getPosition() const
	{
		return Locatable::getPosition();
	}

	void Wall::setPosition(const AbsPosition& _pos)
	{
		throw std::runtime_error("Wall: setPosition is not allowed for walls.");
	}

	std::optional<game::AbsPosition> Wall::getDestination() const
	{
		return Locatable::getDestination();
	}

	DirectionVector Wall::getDirectionVector() const
	{
		return Locatable::getDirectionVector();
	}

	unit::Type Wall::getType() const
	{
		return unit::Type::wall;
	}

	Stat Wall::calculateStat(StatType _type) const
	{
		if (_type == StatType::armor)
			return super::calculateStat(StatType::armor) + _calculateChainArmorBonus();
		return super::calculateStat(_type);
	}

	bool Wall::isInBuildRange(const TilePosition& _pos, const Participant& _owner)
	{
		auto& tileMap = _owner.getGameDependencies().tileMap;
		TilePosition endPos = _pos;
		endPos.setX(endPos.getX() + 2 >= tileMap.getSize().getX() - 1 ? tileMap.getSize().getX() - 1 : endPos.getX() + 2);
		endPos.setY(endPos.getY() + 1 >= tileMap.getSize().getY() - 1 ? tileMap.getSize().getY() - 1 : endPos.getY() + 1);
		for (auto y = _pos.getY() <= 1 ? 0 : _pos.getY() - 1; y <= endPos.getY(); ++y)
		{
			for (auto x = _pos.getX() <= 2 ? 0 : _pos.getX() - 2; x <= endPos.getX(); ++x)
			{
				auto unit = tileMap.get(TilePosition(x, y));
				if (unit != nullptr && &unit->getOwner() == &_owner)
					return true;
			}
		}

		// check bottom line
		auto posX = _pos.getX();
		--posX;
		for (int i = 0; i < 3; ++i, ++posX)
		{
			TilePosition pos(posX, _pos.getY() + 2);
			if (tileMap.isValidPosition(pos))
			{
				auto unit = tileMap.get(pos);
				if (unit != nullptr && &unit->getOwner() == &_owner)
					return true;
			}
		}

		// check top line
		posX = _pos.getX();
		--posX;
		for (int i = 0; i < 3; ++i, ++posX)
		{
			TilePosition pos(posX, _pos.getY() - 2);
			if (tileMap.isValidPosition(pos))
			{
				auto unit = tileMap.get(pos);
				if (unit != nullptr && &unit->getOwner() == &_owner)
					return true;
			}
		}
		return false;
	}

	void Wall::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = WallState{ std::clamp(m_ChainBonus, 0, *getGameDependencies().gameData.gameProperties.game.maxWallChain) };
	}

	void Wall::setPossingTower(TowerHandle _tower)
	{
		m_Tower = _tower;
	}

	bool Wall::canDie() const
	{
		return !(m_Tower && m_Tower->isAlive());
	}

	TowerHandle Wall::getPossingTower() const
	{
		return m_Tower;
	}
} // namespace game::unit
