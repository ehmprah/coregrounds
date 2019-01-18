#pragma once

#include "Unit.hpp"

namespace game
{
namespace unit
{
	struct WallInitializer
	{
		UnitInitializer unitInit;
		const data::Wall& wallData;
		TilePosition pos;
	};

	class Wall :
		public Unit,
		private Locatable
	{
		using super = Unit;

	public:
		const data::Wall& wallData;

		Wall(const WallInitializer& _initializer);

		WallHandle getHandle() const;

		void addIntoWorld() override;
		void removeFromWorld() override;
		void calculateChainBonus();

		bool hasNeighbour() const;

		const AbsShape& getCollider() const override;
		AbsPosition getPosition() const override;
		void setPosition(const AbsPosition& _pos) override;
		std::optional<AbsPosition> getDestination() const override;
		DirectionVector getDirectionVector() const override;
		unit::Type getType() const override;
		bool canDie() const override;

		Stat calculateStat(StatType _type) const override;

		static bool isInBuildRange(const TilePosition& _pos, const Participant& _owner);

		void setPossingTower(TowerHandle _tower);
		TowerHandle getPossingTower() const;

	private:
		sl2::MasterHandle<unit::Wall> m_MasterHandle;
		TowerHandle m_Tower;
	
		int m_ChainBonus = 0;
		bool m_IsInWorld = true;
	
		using ChainInfo = std::pair<int/*count*/, std::vector<Wall*>>;
		ChainInfo _createChainInfo() const;
		void _setupChainBonus(ChainInfo _walls);
		void _clearChainBonus();
		void _setPosition(const TilePosition& _pos);
	
		Stat _calculateChainArmorBonus() const;

		void cleanUp() override;
		void setupDerivedState(UnitState& _unitState) const override;
	};
} // namespace unit
} // namespace game
