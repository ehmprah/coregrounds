#pragma once

#include "Unit.hpp"
#include "Locatable.hpp"

namespace game::unit
{
	struct CoreInitializer
	{
		UnitInitializer unitInit;
		const data::Core& coreData;
		TilePosition pos;
	};

	class Core :
		public Unit,
		private Locatable
	{
	public:
		Core(const CoreInitializer& _initializer);

		void setOwner(Participant& _owner) override;

		const AbsShape& getCollider() const override;
		AbsPosition getPosition() const override;
		void setPosition(const AbsPosition& _pos) override;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;
		unit::Type getType() const override;

		bool isErasable() const override; // do never allow core erase

	private:
		using super = Unit;

		const data::Core& m_Data;

		void setupDerivedState(UnitState& _unitState) const override;
	};
	using CorePtr = std::unique_ptr<Core>;
} // namespace game::unit
