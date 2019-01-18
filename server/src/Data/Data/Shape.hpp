#pragma once

#include "Core/SharedDefines.hpp"
#include "PropertyDefines.hpp"

namespace game::data
{
	struct AreaModifier;
	struct Shape
	{
	public:
		struct Circle
		{
			prop::Property<AbsCoordType> radius{ "radius", 0, prop::GreaterZero };
		};

		struct Rect
		{
			prop::Property<AbsCoordType> width{ "width", 0, prop::GreaterZero };
			prop::Property<AbsCoordType> height{ "height", 0, prop::GreaterZero };
		};

	private:
		using Data = std::variant<std::monostate, Rect, Circle>;
		Data m_Data;

		template <class T>
		using IsData = std::disjunction<std::is_same<T, Rect>, std::is_same<T, Circle>>;

		template <class TData, typename = std::enable_if_t<IsData<TData>::value>>
		Data _validateData(TData _data) const
		{
			if constexpr (std::is_same_v<TData, Rect>)
			{
				if (_data.width <= 0 || _data.height <= 0)
				{
					LOG_WARN("Invalid rectangle - width: " << *_data.width << " and height: " << *_data.height << " Set to Null area.");
					return {};
				}
			}
			else if constexpr (std::is_same_v<TData, Circle>)
			{
				if (_data.radius <= 0)
				{
					LOG_WARN("Invalid circle - radius: " << *_data.radius << " Set to Null area.");
					return {};
				}
			}
			else
				static_assert(false, "type not implemented.");
			return std::move(_data);
		}

	public:
		prop::Property<AbsCoordType> x{ "x", 0 };
		prop::Property<AbsCoordType> y{ "y", 0 };

		template <class TData, typename = std::enable_if_t<IsData<TData>::value>>
		void setData(TData _data)
		{
			m_Data = _validateData(std::move(_data));
		}

		template <class TData, typename = std::enable_if_t<IsData<TData>::value>>
		const TData& getData() const
		{
			return std::get<TData>(m_Data);
		}

		template <class TData, typename = std::enable_if_t<IsData<TData>::value>>
		bool hasData() const
		{
			return std::holds_alternative<TData>(m_Data);
		}

		bool isNull() const;

		Shape modify(const AreaModifier& _mod) const;
	};

	struct AreaModifier
	{
		enum class Type
		{
			set,
			mod
		} type = Type::set;

		Shape data;
	};
} // namespace game::data
