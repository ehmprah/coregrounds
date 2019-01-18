#pragma once

template <class T>
class DurationalObject : public std::enable_shared_from_this<DurationalObject<T>>, sl2::NonCopyable
{
private:
	using Lock = std::scoped_lock<std::mutex>;
	using Timer = boost::asio::deadline_timer;

public:
	template <class... Args>
	DurationalObject(Args&&... _args) :
		m_Object(std::forward<Args>(_args)...)
	{
	}

	void setupTimer(TimerFactory& _timerFactory)
	{
		m_Timer = _timerFactory.createTimer(boost::posix_time::seconds(30), std::bind(&DurationalObject<T>::_timeout, this->shared_from_this()));
	}

	T& operator *()
	{
		return m_Object;
	}

	T* operator ->()
	{
		return &m_Object;
	}

private:
	T m_Object;
	std::optional<Timer> m_Timer;

	void _timeout()
	{
	}
};
