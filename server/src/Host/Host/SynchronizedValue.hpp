#pragma once

template <class T>
class SynchronizedValue
{
private:
	using Lock = std::scoped_lock<std::mutex>;

public:
	SynchronizedValue() = default;

	SynchronizedValue(const SynchronizedValue& _other) :
		m_Value(_other.load())
	{}

	SynchronizedValue(SynchronizedValue& _other) :
		SynchronizedValue(static_cast<const SynchronizedValue&>(_other))
	{}

	SynchronizedValue(SynchronizedValue&& _other) :
		m_Value(_other._takeValue())
	{}

	//template <typename... Args, typename = std::enable_if_t<sizeof...(Args) != 1 || !(std::is_convertible_v<Args, LockedValue> || ...)>>
	//LockedValue(Args&&... _args) :
	//	m_Value(std::forward<Args>(_args)...)
	//{}

	template <typename... Args>
	SynchronizedValue(Args&&... _args) :
		m_Value(std::forward<Args>(_args)...)
	{}

	SynchronizedValue& operator =(const SynchronizedValue& _other)
	{
		store(_other.load());
		return *this;
	}

	SynchronizedValue& operator =(SynchronizedValue&& _other)
	{
		store(_other._takeValue());
		return *this;
	}

	template <class U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
	SynchronizedValue& operator =(U&& _value)
	{
		store(std::forward<U>(_value));
		return *this;
	}

	T operator *() const
	{
		return load();
	}

	template <class U>
	void store(U&& _value)
	{
		Lock lock(m_Mutex);
		m_Value = std::forward<U>(_value);
	}

	T load() const
	{
		Lock lock(m_Mutex);
		return m_Value;
	}

private:
	mutable std::mutex m_Mutex;
	T m_Value;

	T _takeValue()
	{
		Lock lock(m_Mutex);
		return std::move(m_Value);
	}
};
