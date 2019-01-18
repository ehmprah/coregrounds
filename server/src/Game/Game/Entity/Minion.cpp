#include "stdafx.h"
#include "Minion.hpp"
#include "Map/RoutingMap.hpp"
#include "CollisionDetector.hpp"
#include "Participant.hpp"
#include "Data/Game.hpp"
#include "Data/Unit.hpp"
#include "Core.hpp"
#include "Spell/AuraEvent.hpp"
#include "Core/HelperFunctions.hpp"
#include "Shape.hpp"

namespace game::unit
{
	/*#####
	# PathMovementGenerator
	#####*/
	PathMovementGenerator::PathMovementGenerator(const Minion& _owner, const map::RoutingMap& _routingMap) :
		m_Map(_routingMap),
		m_Owner(_owner)
	{
	}

	std::optional<AbsPosition> PathMovementGenerator::getNext(const AbsPosition& _curPos)
	{
		if (auto nextDirection = _calculateNextDirection(_curPos))
		{
			m_Direction = nextDirection->first;
			m_IgnoreCollision = false;
			return nextDirection->second;
		}
		return std::nullopt;
	}

	std::optional<AbsPosition> PathMovementGenerator::recalculateMovement(const AbsPosition& _curPos)
	{
		if (auto nextDirection = _calculateNextDirection(_curPos))
		{
			// We can check here, if our current direction is the opposite of the new direction.
			// If this happens, ignore collision for next movement.
			m_IgnoreCollision = m_Direction == Direction::left && nextDirection->first == Direction::right ||
				m_Direction == Direction::right && nextDirection->first == Direction::left ||
				m_Direction == Direction::up && nextDirection->first == Direction::down ||
				m_Direction == Direction::down && nextDirection->first == Direction::up;
			m_Direction = nextDirection->first;
			return nextDirection->second;
		}
		return std::nullopt;
	}

	bool PathMovementGenerator::ignoreCollision() const
	{
		return m_IgnoreCollision || m_Owner.hasUnitFlag(Flags::unsolid);
	}

	int PathMovementGenerator::_getPossibleDirections(const AbsPosition& _curPos) const
	{
		int result = 0;
		if (isAxisAligned(_curPos.getX()))
			result |= (int)Direction::up | (int)Direction::down;
		if (isAxisAligned(_curPos.getY()))
			result |= (int)Direction::left | (int)Direction::right;
		return result;
	}

	std::optional<std::pair<Direction, AbsPosition>> PathMovementGenerator::_calculateNextDirection(const AbsPosition& _pos) const
	{
		auto possibleDirs = _getPossibleDirections(_pos);

		assert(m_Map.getWidth() > 0 && m_Map.getHeight() > 0);
		auto tilePos = mapToTilePosition(_pos);
		if (possibleDirs == 0 || !map::isInRange(m_Map, tilePos) || (m_Map.get(tilePos) == 0 && isTileCenter(_pos)))
			return std::nullopt;

		using pair = std::pair<std::pair<Direction, AbsPosition>, int>;
		std::vector<pair> direction;
		for (int i = 0; i < 4; ++i)
		{
			auto pos = mapToTilePosition(_pos);
			switch (i)
			{
			case 0:		// left
			{
				auto otherField = _pos.getX() - std::floor(pos.getX()) <= TileAbsoluteSize / 2 + Epsilon;
				if (possibleDirs & (int)Direction::left && (!otherField || pos.getX() > 0))
				{
					if (otherField)
						pos.setX(pos.getX() - 1);
					if (auto val = m_Map.get(pos); val >= 0)
						direction.emplace_back(std::make_pair(Direction::left, mapToAbsPosition(pos) + TileAbsoluteSize / 2), val);
				}
				break;
			}
			case 1:		// right
			{
				auto otherField = _pos.getX() - std::floor(pos.getX()) >= TileAbsoluteSize / 2 - Epsilon;
				if (possibleDirs & (int)Direction::right && (!otherField || pos.getX() < m_Map.getWidth() - 1))
				{
					if (otherField)
						pos.setX(pos.getX() + 1);
					if (auto val = m_Map.get(pos); val >= 0)
						direction.emplace_back(std::make_pair(Direction::right, mapToAbsPosition(pos) + TileAbsoluteSize / 2), val);
				}
				break;
			}
			case 2:		// top
			{
				auto otherField = _pos.getY() - std::floor(pos.getY()) <= TileAbsoluteSize / 2 + Epsilon;
				if (possibleDirs & (int)Direction::up && (!otherField || pos.getY() > 0))
				{
					if (otherField)
						pos.setY(pos.getY() - 1);
					if (auto val = m_Map.get(pos); val >= 0)
						direction.emplace_back(std::make_pair(Direction::up, mapToAbsPosition(pos) + TileAbsoluteSize / 2), val);
				}
				break;
			}
			case 3:		// down
			{
				auto otherField = _pos.getY() - std::floor(pos.getY()) >= TileAbsoluteSize / 2 - Epsilon;
				if (possibleDirs & (int)Direction::down && (!otherField || pos.getY() < m_Map.getHeight() - 1))
				{
					if (otherField)
						pos.setY(pos.getY() + 1);
					if (auto val = m_Map.get(pos); val >= 0)
						direction.emplace_back(std::make_pair(Direction::down, mapToAbsPosition(pos) + TileAbsoluteSize / 2), val);
				}
				break;
			}
			}
		}
		if (!direction.empty())
		{
			std::sort(std::begin(direction), std::end(direction),
				[](const auto& _lhs, const auto& _rhs) { return _lhs.second < _rhs.second; }
			);
			auto minValue = direction.front().second;
			auto iMax = std::distance(std::begin(direction), std::find_if_not(std::begin(direction), std::end(direction),
				[minValue](const auto& _other) { return _other.second == minValue; }
			));
			return direction[randomInt<std::size_t>(0, iMax - 1)].first;
		}
		return std::nullopt;
	}

	/*#####
	# Minion
	#####*/
	Minion::Minion(const MinionInitializer& _initializer) :
		Movable(createShape(_initializer.minionData.getBody())),
		Attackable(_initializer.minionData, _initializer.targetPreference, *this, getGameDependencies()),
		super(_initializer.unitInit),
		m_Data(_initializer.minionData)
	{
		setCollisionDetector(std::make_unique<BaseCollisionDetector>(getGameDependencies().colliderMap, *this));
		setUnitFlag(Flags::unsolid, m_Data.getFlags().contains(MinionFlag::unsolid));
	}

	void Minion::applyUnitCondition(Condition _condition, bool _set)
	{
		if (_set)
		{
			switch (_condition)
			{
			case unit::Condition::daze:
				restartAttack();
				break;
			}
		}
		super::applyUnitCondition(_condition, _set);
	}

	void Minion::setOwner(Participant& _owner)
	{
		auto& oldOwner = getOwner();
		super::setOwner(_owner);
		if (&oldOwner != &_owner)
		{
			setMovementGenerator(std::make_unique<unit::PathMovementGenerator>(*this, _owner.getOpponent().getRoutingMap()));
			recalculateMovement();
		}
	}

	void Minion::update(Time _diff)
	{
		super::update(_diff);
		if (isDead())
			return;

		if (!hasCondition(*this, Condition::daze))
			updateAttackable(_diff);

		if (!hasCondition(*this, Condition::root))
		{
			updateMovement(_diff);
			// cast suicide spell if destination core reached
			auto& opponentCore = getOwner().getOpponent().getCore();
			if (getCollider().contains(opponentCore.getCollider().getCenter()))
			{
				auto& gameData = getGameDependencies().gameData;
				if (auto spell = data::findData(gameData.spells, *gameData.gameProperties.game.minionCoreSuicideSpellID))
				{
					cast(*spell, opponentCore.getCastableHandle());
					kill();
				}
			}
		}
	}

	const AbsShape& Minion::getCollider() const
	{
		return Movable::getArea();
	}

	AbsPosition Minion::getPosition() const
	{
		return Movable::getPosition();
	}

	void Minion::setPosition(const AbsPosition& _pos)
	{
		if (_pos != getPosition())
		{
			assert(std::abs(_pos.getX() - std::floor(_pos.getX()) - 0.5) <= Epsilon || std::abs(_pos.getY() - std::floor(_pos.getY()) - 0.5) <= Epsilon);
			Locatable::setPosition(_pos);
			super::adjustCollider();
		}
	}

	std::optional<AbsPosition> Minion::getDestination() const
	{
		return Movable::getDestination();
	}

	DirectionVector Minion::getDirectionVector() const
	{
		if (auto target = getTarget();
			target && target->isUnit() && target->toUnit().getGUID() != getGUID())
		{
			auto& unit = target->toUnit();
			return sl2::createNormalized(unit.getPosition() - getPosition());
		}
		return Movable::getDirectionVector();
	}

	Type Minion::getType() const
	{
		return Type::minion;
	}

	Stat Minion::getSpeed() const
	{
		return getUnitState().stats.getStat(StatType::moveSpeed);
	}

	AbsCircle Minion::getAttackArea() const
	{
		return AbsCircle(getPosition(), getUnitState().stats.getStat(StatType::attackRange));
	}

	const TargetPreference* Minion::getTargetPreference() const
	{
		return Attackable::getTargetPreference();
	}

	std::optional<AbsCoordType> Minion::getDistanceToDestination() const
	{
		return Movable::getDistanceToDestination();
	}

	void Minion::recalculateMovement()
	{
		Movable::recalculateMovement();
	}

	void Minion::setMovementGenerator(MovementGeneratorPtr _movementGenerator)
	{
		Movable::setMovementGenerator(std::move(_movementGenerator));
		Movable::startMovement();
	}

	void Minion::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = MinionState{ getAttackProgressPercent(), getDirectionVector() };
	}
} // namespace game::unit
