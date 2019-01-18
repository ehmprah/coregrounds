#pragma once

#include "Unit.hpp"
#include "Attackable.hpp"

namespace game::unit
{
	class TowerTimer
	{
	public:
		TowerTimer(const device::TowerFactory& _towerFactory);

		void registerTower(TowerHandle _handle);
		void update(Time _diff);

		const Timer& getBuildCooldown() const;
		const Timer& getUpgradeCooldown() const;
		const Timer& getWindupCooldown() const;
		const Timer& getMoveCooldown() const;
		const Timer& getDeathCooldown() const;

		void emitBuildCooldown(Time _duration);
		void emitUpgradeCooldown(Time _duration);
		void emitWindupCooldown();
		void emitMoveCooldown();
		void clearMoveCooldown();
		void emitDeathCooldown();

	private:
		const device::TowerFactory& m_TowerFactory;
		const data::Tower& m_TowerData;
		TowerHandle m_TowerHandle;
		std::optional<Time> m_LastUpgradeOrBuildTime;
		Timer m_BuildTimer;
		Timer m_UpgradeTimer;
		Timer m_DeathTimer;
		Timer m_WindupTimer;
		Timer m_MoveTimer;
	};

	Time getElapsedTime(const TowerTimer& _timer);
	Time getRemainingTime(const TowerTimer& _timer);
	Time getTotalTime(const TowerTimer& _timer);
	bool hasCooldown(const TowerTimer& _timer);
	float getProgressInPercent(const TowerTimer& _timer);

	struct TowerInitializer
	{
		UnitInitializer unitInit;
		const data::Tower& towerData;
		const TargetPreference* targetPreference;
		TowerTimer& cooldownTimer;
	};

	class Tower :
		public Unit,
		public Attackable,
		private Locatable
	{
		using super = Unit;

	public:
		Tower(const TowerInitializer& _initializer);

		virtual void applyUnitCondition(Condition _condition, bool _set = true) override;

		void setOwner(Participant& _owner) override;

		Stat calculateVisibilityRange() const override;

		const data::Tower& getData() const;
		TowerHandle getHandle() const;

		WallHandle getWall() const;

		void addIntoWorld() override;
		void removeFromWorld() override;

		bool isValidPosition(const TilePosition& _pos) const;
		bool isInMovableRange(const TilePosition& _pos) const;
		bool isMovablePosition(const TilePosition& _pos) const;

		void activateMove(const TilePosition& _pos);
		const TowerTimer& getCooldownTimer() const;

		void update(Time _diff) override;

		const AbsShape& getCollider() const override;
		AbsPosition getPosition() const override;
		void setPosition(const AbsPosition& _pos) override;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;
		unit::Type getType() const override;
		AbsCircle getAttackArea() const override;

		const TargetPreference* getTargetPreference() const override;

		static bool isValidPosition(const map::TileMap& _tileMap, const TilePosition& _pos, const Participant& _owner);

	private:
		sl2::MasterHandle<Tower> m_MasterHandle;

		const data::Tower& m_Data;
		TowerTimer& m_TowerTimer;
		WallHandle m_Wall;
		WallHandle m_DestinationWall;

		Level* m_XP = nullptr;

		void _finalizeMove();

		void setupDerivedState(UnitState& _unitState) const override;
		void derivedFinalizeTick() override;
	};
} // namespace game::unit
