#pragma once

#include "Core/SharedDefines.hpp"

namespace game
{
namespace detail
{
	template <class Type, typename = std::enable_if_t<std::is_enum_v<Type>>>
	std::size_t _mapToIndex(Type _type)
	{
		return static_cast<std::size_t>(_type) - 1;
	}
} // namespace detail

	template <class Type, class ValueType = Stat>
	class StatContainer
	{
	private:
		std::array<ValueType, static_cast<std::size_t>(Type::max) - 1> m_Stats{};

	public:
		ValueType getStat(Type _type) const
		{
			return m_Stats[detail::_mapToIndex(_type)];
		}

		void setStat(Type _type, ValueType _value)
		{
			m_Stats[detail::_mapToIndex(_type)] = _value;
		}
	};
} // namespace game
