#pragma once

#include "AI\Utility.hpp"

namespace game::ai 
{
	enum class HeatmapType 
	{
		Damage,
		Healing,
	};

	class HeatMap
	{
	public:
		game::TilePosition getThreatPoint() const;
		std::optional<game::AbsPosition> getHotSpot(HeatmapType _type, int _minHeat = 1) const;
		std::optional<TilePosition> getHotTile(HeatmapType _type, int _minHeat = 0) const;
		int heatAtTile(HeatmapType _type, game::TilePosition _pos) const;
		int heatAroundTile(HeatmapType _type, game::TilePosition _pos, int _range = 1) const;

		void update(const protobuf::out::Match_Game& _output);

	private:
		std::array<sl2::container::Vector2D<int>, 2> m_Maps;

		const sl2::container::Vector2D<int>& getHeatmap(HeatmapType _type) const;
		void buildDamageMap(const protobuf::out::Match_Game& _output);
		void buildHealingMap(const protobuf::out::Match_Game& _output);
	};

	void addHeat(sl2::container::Vector2D<int>& _map, const TilePosition& _position, bool _extra);
}