#include "stdafx.h"
#include "Tower.hpp"
#include "Participant.hpp"
#include "Wall.hpp"
#include "Data/Game.hpp"
#include "Data/Unit.hpp"
#include "Map/ColliderMap.hpp"
#include "Map/TileMap.hpp"
#include "Shape.hpp"

namespace game::unit
{
	const data::Tower& _findTowerData(const device::TowerFactory& _towerFactory)
	{
		auto& facData = _towerFactory.getFactoryData().getData<data::Factory::Tower>();
		auto dataPtr = data::findData(_towerFactory.getGameDependencies().gameData.towers, *facData.id);
		assert(dataPtr);
		return *dataPtr;
	}

	Stat _getMovementSpeed(const TowerHandle& _handle)
	{
		if (_handle && _handle->isAlive())
		{
			return _handle->getUnitState().stats.getStat(StatType::moveSpeed);
		}
		return 1;
	}

	Time _getDeathCooldownDuration(const GameDependencies& _gameDependencies)
	{
		return *_gameDependencies.gameData.gameProperties.game.towerDeathCooldown;
	}

	TowerTimer::TowerTimer(const device::TowerFactory& _towerFactory) :
		m_TowerFactory(_towerFactory),
		m_TowerData(_findTowerData(_towerFactory))
	{
	}

	void TowerTimer::update(Time _diff)
	{
		auto moveSpeed = _getMovementSpeed(m_TowerHandle);
		auto cooldownSpeed = device::getStat(m_TowerFactory, device::StatType::cooldownSpeed);
		_diff = updateTimerRelatedToStat(m_WindupTimer, _diff, *m_TowerData.movementWindup, moveSpeed);
		_diff = updateTimerRelatedToStat(m_MoveTimer, _diff, *m_TowerData.movementCooldown, moveSpeed);
		if (m_LastUpgradeOrBuildTime)
		{
			_diff = updateTimerRelatedToStat(m_UpgradeTimer, _diff, *m_LastUpgradeOrBuildTime, cooldownSpeed);
			_diff = updateTimerRelatedToStat(m_BuildTimer, _diff, *m_LastUpgradeOrBuildTime, cooldownSpeed);
		}
		_diff = updateTimerRelatedToStat(m_DeathTimer, _diff, _getDeathCooldownDuration(m_TowerFactory.getGameDependencies()), cooldownSpeed);
	}

	void TowerTimer::registerTower(TowerHandle _handle)
	{
		m_TowerHandle = std::move(_handle);
	}

	void TowerTimer::emitBuildCooldown(Time _duration)
	{
		assert(!m_BuildTimer.isActive());
		auto cooldownSpeed = device::getStat(m_TowerFactory, device::StatType::cooldownSpeed);
		_duration /= std::max(cooldownSpeed, std::numeric_limits<Stat>::epsilon());
		m_LastUpgradeOrBuildTime = _duration;
		m_BuildTimer.start(_duration);
	}

	void TowerTimer::emitUpgradeCooldown(Time _duration)
	{
		assert(!m_UpgradeTimer.isActive());
		auto cooldownSpeed = device::getStat(m_TowerFactory, device::StatType::cooldownSpeed);
		_duration /= std::max(cooldownSpeed, std::numeric_limits<Stat>::epsilon());
		m_LastUpgradeOrBuildTime = _duration;
		m_UpgradeTimer.start(_duration);
	}

	void TowerTimer::emitWindupCooldown()
	{
		assert(!m_WindupTimer.isActive());
		auto speed = _getMovementSpeed(m_TowerHandle);
		auto duration = *m_TowerData.movementWindup / std::max(speed, std::numeric_limits<Stat>::epsilon());
		m_WindupTimer.start(duration);
	}

	void TowerTimer::emitMoveCooldown()
	{
		assert(!m_MoveTimer.isActive());
		auto speed = _getMovementSpeed(m_TowerHandle);
		auto duration = *m_TowerData.movementCooldown / std::max(speed, std::numeric_limits<Stat>::epsilon());
		m_MoveTimer.start(duration);
	}

	void TowerTimer::clearMoveCooldown()
	{
		m_MoveTimer.reset();
	}

	void TowerTimer::emitDeathCooldown()
	{
		assert(!m_DeathTimer.isActive());
		m_DeathTimer.start(_getDeathCooldownDuration(m_TowerFactory.getGameDependencies()));
	}

	const Timer& TowerTimer::getBuildCooldown() const
	{
		return m_BuildTimer;
	}

	const Timer& TowerTimer::getUpgradeCooldown() const
	{
		return m_UpgradeTimer;
	}

	const Timer& TowerTimer::getWindupCooldown() const
	{
		return m_WindupTimer;
	}

	const Timer& TowerTimer::getMoveCooldown() const
	{
		return m_MoveTimer;
	}

	const Timer& TowerTimer::getDeathCooldown() const
	{
		return m_DeathTimer;
	}

	template <class TFunc>
	void _forEachTimer(const TowerTimer& _timer, TFunc _func)
	{
		_func(_timer.getBuildCooldown());
		_func(_timer.getDeathCooldown());
		_func(_timer.getMoveCooldown());
		_func(_timer.getUpgradeCooldown());
		_func(_timer.getWindupCooldown());
	}

	Time getElapsedTime(const TowerTimer& _timer)
	{
		auto time = Time::zero();
		_forEachTimer(_timer,
			[&time](auto& _timer)
			{
				if (_timer.isActive())
					time += std::chrono::duration_cast<Time>(_timer.getTimeElapsed());
			}
		);
		return time;
	}

	Time getRemainingTime(const TowerTimer& _timer)
	{
		auto time = Time::zero();
		_forEachTimer(_timer,
			[&time](auto& _timer)
			{
				if (_timer.isActive())
					time += std::chrono::duration_cast<Time>(_timer.getTimerValue());
			}
		);
		return time;
	}

	Time getTotalTime(const TowerTimer& _timer)
	{
		auto time = Time::zero();
		_forEachTimer(_timer,
			[&time](auto& _timer)
			{
				if (_timer.isActive())
					time += std::chrono::duration_cast<Time>(_timer.getTimerTotal());
			}
		);
		return time;
	}

	bool hasCooldown(const TowerTimer& _timer)
	{
		bool active = false;
		_forEachTimer(_timer, [&active](auto& _timer) { active |= _timer.isActive(); });
		return active;
	}

	float getProgressInPercent(const TowerTimer& _timer)
	{
		if (hasCooldown(_timer))
		{
			auto total = getTotalTime(_timer);
			return 0 < total.count() ? getElapsedTime(_timer) * 100.f / total : 0.f;
		}
		return 0.f;
	}

	void _removeWall(Wall& _wall, map::TileMap& _tileMap, map::ColliderMap& _colliderMap)
	{
		_tileMap.remove(_wall);
		//_colliderMap.removeUnit(_wall);
	}

	void _addWall(Wall& _wall, map::TileMap& _tileMap, map::ColliderMap& _colliderMap)
	{
		_tileMap.set(_wall);
		//_map.addUnit(_wall);
	}

	Tower::Tower(const TowerInitializer& _initializer) :
		Locatable(createShape(_initializer.towerData.getBody())),
		Attackable(_initializer.towerData, _initializer.targetPreference, *this, getGameDependencies()),
		super(_initializer.unitInit),
		m_Data(_initializer.towerData),
		m_MasterHandle(*this),
		m_TowerTimer(_initializer.cooldownTimer)
	{
	}

	void Tower::applyUnitCondition(Condition _condition, bool _set)
	{
		if (_set)
		{
			switch (_condition)
			{
			case unit::Condition::daze:
				restartAttack();
				break;
			case unit::Condition::root:
				break;
			}
		}
		super::applyUnitCondition(_condition, _set);
	}

	void Tower::setOwner(Participant& _owner)
	{
		if (&getOwner() != &_owner && m_Wall)
			m_Wall->setOwner(_owner);
		super::setOwner(_owner);
	}

	Stat Tower::calculateVisibilityRange() const
	{
		if (m_Wall)
			return std::max(super::calculateVisibilityRange(), m_Wall->calculateVisibilityRange());
		return super::calculateVisibilityRange();
	}

	TowerHandle Tower::getHandle() const
	{
		return m_MasterHandle.get();
	}

	const data::Tower& Tower::getData() const
	{
		return m_Data;
	}

	sl2::Handle<Wall> Tower::getWall() const
	{
		return m_Wall;
	}

	void Tower::addIntoWorld()
	{
		if (!isInWorld())
		{
			auto ptr = getGameDependencies().tileMap.get(mapToTilePosition(getPosition()));
			if (!ptr || ptr->getType() != Type::wall)
				return;
			m_Wall = static_cast<Wall&>(*ptr).getHandle();
			m_Wall->removeFromWorld();
			m_Wall->setPossingTower(getHandle());
			getGameDependencies().tileMap.set(*this);

			// we removed the wall from the world, thus the chain bonus isn't calculated correctly. We have to
			// force a recalculation now.
			m_Wall->calculateChainBonus();
		}
		super::addIntoWorld();
	}

	void Tower::removeFromWorld()
	{
		if (isInWorld())
		{
			// if the tower is already placed on a wall, we remove tower first from tile map and
			// add the wall to the tile map again. This can't occur if the tower just gets created.
			if (m_Wall)
			{
				getGameDependencies().tileMap.remove(*this);
				m_Wall->addIntoWorld();
				m_Wall->setPossingTower(TowerHandle());
			}
		}
		super::removeFromWorld();
	}

	bool Tower::isValidPosition(const map::TileMap& _tileMap, const TilePosition& _pos, const Participant& _owner)
	{
		if (_tileMap.isValidPosition(_pos))
		{
			auto unit = _tileMap.get(_pos);
			return unit && &unit->getOwner() == &_owner && unit->getType() == unit::Type::wall && unit->isAlive();
		}
		return false;
	}

	bool Tower::isValidPosition(const TilePosition& _pos) const
	{
		if (isValidPosition(getGameDependencies().tileMap, _pos, getOwner()))
			return m_Wall ? getGameDependencies().tileMap.get(_pos) != m_Wall.get() : true;
		return false;
	}

	bool Tower::isInMovableRange(const TilePosition& _pos) const
	{
		return sl2::geometry2d::contains(AbsCircle(getPosition(), *getData().movementRange), mapToAbsPosition(_pos) + TileAbsoluteSize / 2);
	}

	bool Tower::isMovablePosition(const TilePosition& _pos) const
	{
		return isInMovableRange(_pos) && isValidPosition(_pos);
	}

	void Tower::activateMove(const TilePosition& _pos)
	{
		if (!hasCooldown(m_TowerTimer) && isMovablePosition(_pos))
		{
			auto& wall = static_cast<Wall&>(*getGameDependencies().tileMap.get(_pos));
			m_DestinationWall = wall.getHandle();
			m_TowerTimer.emitWindupCooldown();
			m_TowerTimer.emitMoveCooldown();
		}
	}

	void Tower::_finalizeMove()
	{
		auto isValid = [](auto _handle) { return _handle && unit::isAlive(*_handle) && unit::isInWorld(*_handle); };

		if (!hasCondition(*this, Condition::root) && isValid(m_DestinationWall) &&
			belongToSameOwner(*m_DestinationWall, *this) && !isValid(m_DestinationWall->getPossingTower()))
		{
			setPosition(m_DestinationWall->getPosition());
		}
		else
		{
			m_TowerTimer.clearMoveCooldown();
		}
		m_DestinationWall.reset();
	}

	const TowerTimer& Tower::getCooldownTimer() const
	{
		return m_TowerTimer;
	}

	void Tower::update(Time _diff)
	{
		super::update(_diff);
		if (isDead())
			return;

		if (hasCooldown(m_TowerTimer))
		{
			if (m_TowerTimer.getWindupCooldown().getState() == Timer::State::justFinished)
				_finalizeMove();
		}
		else if (!hasCondition(*this, Condition::daze))
			updateAttackable(_diff);
	}

	const AbsShape& Tower::getCollider() const
	{
		return Locatable::getArea();
	}

	AbsPosition Tower::getPosition() const
	{
		return Locatable::getPosition();
	}

	void Tower::setPosition(const AbsPosition& _pos)
	{
		auto tilePos = mapToTilePosition(_pos);
		if (!isValidPosition(tilePos))
			return;

		auto ptr = getGameDependencies().tileMap.get(tilePos);
		if (!ptr || ptr->getType() != Type::wall)
			return;

		auto absPos = mapToAbsPosition(tilePos) + TileAbsoluteSize / 2;
		if (absPos != getPosition())
		{
			removeFromWorld();
			Locatable::setPosition(absPos);
			addIntoWorld();
			super::adjustCollider();
			LOG_DEBUG("Tower GUID: " << getGUID() << " relocated to " << _pos.getX() << "/" << _pos.getY());
		}
	}

	std::optional<AbsPosition> Tower::getDestination() const
	{
		return Locatable::getDestination();
	}

	DirectionVector Tower::getDirectionVector() const
	{
		if (auto target = getTarget();
			target && target->isUnit() && target->toUnit().getGUID() != getGUID())
		{
			auto& unit = target->toUnit();
			return sl2::createNormalized(unit.getPosition() - getPosition());
		}
		return Locatable::getDirectionVector();
	}

	Type Tower::getType() const
	{
		return Type::tower;
	}

	AbsCircle Tower::getAttackArea() const
	{
		return AbsCircle(getPosition(), getUnitState().stats.getStat(StatType::attackRange));
	}

	const TargetPreference* Tower::getTargetPreference() const
	{
		return Attackable::getTargetPreference();
	}

	void Tower::setupDerivedState(UnitState& _unitState) const
	{
		_unitState.derivedState = TowerState{ getAttackProgressPercent(), getDirectionVector() };
	}

	void Tower::derivedFinalizeTick()
	{
		super::derivedFinalizeTick();

		if (getUnitState().state == unit::State::justDied)
			m_TowerTimer.emitDeathCooldown();
	}
} // namespace game::unit
