#pragma once

#include "Core/log.hpp"

namespace game::prop
{
	template <class T>
	class _BaseProperty :
		sl2::operators::TotalCompare<_BaseProperty<T>>,
		sl2::operators::TotalCompare2<_BaseProperty<T>, T>
	{
	private:
		T m_Value;
		std::string m_Name;

		template <class T1>
		static T _toT(T1&& _value)
		{
			static_assert(std::is_convertible_v<T1, T>, "T1 must be implicit convertible to T");
			return std::forward<T>(static_cast<T>(_value));
		}

	protected:
		constexpr _BaseProperty(std::string _name) :
			m_Name(std::move(_name))
		{
		}

		template <class T1>
		constexpr _BaseProperty(std::string _name, T1&& _default) :
			m_Value(_toT(std::forward<T1>(_default))),
			m_Name(std::move(_name))
		{}

		~_BaseProperty() = default;
		constexpr _BaseProperty& operator =(const _BaseProperty& _other) = default;

		template <class T1>
		constexpr void setValue(T1&& _value)
		{
			m_Value = _toT(std::forward<T1>(_value));
		}

	public:
		using ValueType = T;

		constexpr std::string_view getName() const
		{
			return m_Name;
		}

		constexpr const T& getValue() const
		{
			return m_Value;
		}

		constexpr const T& operator *() const
		{
			return m_Value;
		}

		constexpr const T* operator ->() const
		{
			return &m_Value;
		}
	};

	template <class T, class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	constexpr bool operator ==(const _BaseProperty<T>& _lhs, const U& _rhs)
	{
		return _lhs.getValue() == _rhs;
	}

	template <class T>
	constexpr bool operator ==(const _BaseProperty<T>& _lhs, const _BaseProperty<T>& _rhs)
	{
		return _lhs.getValue() == _rhs.getValue();
	}

	template <class T, class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	constexpr bool operator <(const _BaseProperty<T>& _lhs, const U& _rhs)
	{
		return _lhs.getValue() < _rhs;
	}

	template <class T, class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	constexpr bool operator <(const U& _lhs, const _BaseProperty<T>& _rhs)
	{
		return _lhs < _rhs.getValue();
	}

	template <class T>
	constexpr bool operator <(const _BaseProperty<T>& _lhs, const _BaseProperty<T>& _rhs)
	{
		return _lhs.getValue() < _rhs.getValue();
	}

	template <class T>
	class Property : public _BaseProperty<T>
	{
	private:
		using super = _BaseProperty<T>;

		using Validator = std::function<bool(const T& _value)>;
		Validator m_Validator;

	public:
		constexpr Property(std::string _name) : 
			super(std::move(_name))
		{}

		template <class T1>
		constexpr Property(std::string _name, T1&& _default, Validator _validator = nullptr) :
			super(std::move(_name), std::forward<T1>(_default)),
			m_Validator(_validator)
		{}

		template <class T1>
		constexpr void setValue(T1&& _value)
		{
			if (!m_Validator || m_Validator(_value))
				super::setValue(std::forward<T1>(_value));
			else
				LOG_ERR("Property name: \"" << super::getName() << "\" invalid value: " << _value << " Reject changes. current value: " << super::getValue());
		}

		template <class T1, typename = std::enable_if_t<std::is_convertible_v<T1, T>>>
		constexpr Property& operator =(T1&& _value)
		{
			setValue(std::forward<T1>(_value));
			return *this;
		}

		constexpr Property& operator =(const Property& _other) = default;
	};
} // namespace game::prop
