#pragma once

namespace game::map
{
	class TileMap :
		sl2::NonCopyable
	{
	private:
		using Field = sl2::container::Vector2D<unit::Unit*>;
		Field m_Field;

		std::unordered_set<unit::Unit*> m_Units;

	public:
		using Size = typename Field::Size;
		TileMap(const Size& _tileMapSize);

		bool isValidPosition(const TilePosition& _pos) const;
		const Size& getSize() const;

		unit::Unit* get(const TilePosition& _tile) const;
		void set(unit::Unit& _unit);
		void remove(unit::Unit& _unit);
	};
} // namespace game::map
