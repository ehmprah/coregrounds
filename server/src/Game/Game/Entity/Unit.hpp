#pragma once

#include "Entity.hpp"
#include "GUIDManager.hpp"
#include "Game/Map/GridMap.hpp"
#include "UnitState.hpp"

namespace game::unit
{
	struct UnitInitializer
	{
		EntityInitializer entityInit;
		Level* level;
		const data::Unit& unitData;
	};

	class Unit :
		public Entity
	{
		friend class World;

	public:
		const data::Unit& unitData;
		using StatModifier = StatModifier<StatType>;
		StatModifier statModifier;

		Unit(const UnitInitializer& _initializer);
		virtual ~Unit() = default;

		const UnitState& getUnitState() const;

		void setOwner(Participant& _owner) override;
		UnitHandle getUnitHandle() const;

		virtual void addIntoWorld();
		virtual void removeFromWorld();

		virtual Stat calculateStat(StatType _type) const;
		virtual Stat getStatBase(StatType _type) const;
		virtual Stat calculateVisibilityRange() const;

		void setHealthPercent(Stat _percent);
		const Team& getTeam() const;
		ID getId() const;
		GUID getGUID() const;
		virtual Type getType() const = 0;
		virtual Level* getLevel() const { return nullptr; };
		virtual const TargetPreference* getTargetPreference() const;
		virtual	std::optional<AbsPosition> getDestination() const = 0;

		bool hasUnitFlag(Flags _flag) const;

		virtual void applyUnitCondition(Condition _condition, bool _set = true);

		bool isAlive() const;
		bool isDead() const;
		virtual bool isErasable() const;
		bool isCrowdControlled() const;
		bool isTargetable() const;
		virtual bool canCast() const override;
		virtual bool canDie() const;

		void kill(const CastableSnapshot& _killerInfo);
		void kill();
		Stat applyHealing(Stat _baseHealing, const CastableSnapshot& _dealerInfo);
		Stat applyDamage(Stat _baseDamage, const CastableSnapshot& _dealerInfo, bool _ignoreLifeLinkAndThorns = false);
		void applyDamageWithoutModifiers(Stat _damage, const CastableSnapshot& _dealerInfo);

		Minion* spawnMinion(ID _id, const AbsPosition& _pos);
		Trigger* spawnTrigger(ID _id, const AbsPosition& _pos);

		virtual void update(Time _diff) override;

		bool isParticipant() const final;
		bool isUnit() const final;
		const Unit& toUnit() const final;
		Unit& toUnit() final;
		bool isDevice() const final;
		const device::Device& toDevice() const final;
		device::Device& toDevice() final;

		virtual DirectionVector getDirectionVector() const = 0;
		virtual AbsPosition getPosition() const = 0;
		virtual const AbsShape& getCollider() const = 0;

	protected:
		bool isInWorld() const;
		void setUnitFlag(Flags _flag, bool _set = true);
		virtual void cleanUp();
		void adjustCollider();
		void applyDirectDamage(Stat _damage);
		void setupCastableSnapshot(CastableSnapshot& _info) const override;

		virtual void derivedFinalizeTick() override;
		virtual void derivedStartPlay() override;
		virtual void setupDerivedState(UnitState& _unitState) const = 0;

	private:
		using super = Entity;
	
		sl2::MasterHandle<Unit> m_MasterHandle;

		GUID m_GUID = 0;

		bool m_IsInWorld = false;

		Level* m_Level = nullptr;
	
		map::GridEntity* m_ColliderGridEntity = nullptr;

		Timer m_RegenerationTimer;
		Stat m_Health;
		bool m_Killed = false;

		std::optional<CastableSnapshot> m_LastAttackerInfo;

		sl2::Bitmask<Flags> m_Flags;
		ConditionHolder m_Conditions{};

		State m_State = State::justSpawned;

		UnitState m_UnitState;
	
		void _applyDealerXpAndStatistics(Stat _totalDamage, const CastableSnapshot& _dealerInfo);
		Stat _shareDmgWithLifePool(const LifeLinkPool& _lifeLinkPool, Stat _damage, const CastableSnapshot& _dealerInfo);
		Stat _getDamageModifier(const data::UnitModifiers& _mods) const;
		void _applyInitAuras();
		void _startRegenerationTimer();
		void _updateRegeneration(Time _diff);
		void _storeUnitState();
	};

	float getHealthPercent(const UnitState& _unitState);
	bool hasCondition(const Unit& _unit, Condition _condition);

	bool isAlive(const Unit& _unit);
	bool isDead(const Unit& _unit);
	bool isInWorld(const Unit& _unit);
	bool belongToSameOwner(const Unit& _lhs, const Unit& _rhs);
	bool isFriendly(const Unit& _unit, const Unit& _other);
	bool isFriendly(const Unit& _unit, const Participant& _participant);
	bool isHostile(const Unit& _unit, const Unit& _other);
	bool isHostile(const Unit& _unit, const Participant& _participant);
	
	unit::Unit& toUnit(CastableHandle& _castable);
} // namespace game::unit
