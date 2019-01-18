#pragma once

#include "Unit.hpp"

namespace game::unit
{
	struct TriggerInitializer
	{
		UnitInitializer unitInit;
		const data::Trigger& triggerData;
	};

	class Trigger :
		public Unit,
		private Locatable
	{
		using super = Unit;

	public:
		const data::Trigger& triggerData;

		Trigger(const TriggerInitializer& _initializer);

		void update(Time _diff) override;
		Type getType() const override;

		const AbsShape& getCollider() const override;
		void setPosition(const AbsPosition& _pos) override;
		AbsPosition getPosition() const override;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;

	private:
		Timer m_LifeTimer;

		void setupDerivedState(UnitState& _unitState) const override;
	};
} // namespace game::unit
