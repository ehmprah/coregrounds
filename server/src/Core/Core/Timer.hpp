#pragma once

#include "Core/StdInclude.hpp"
#include "Core/SLInclude.hpp"

class Timer
{
public:
	enum class State
	{
		none,
		inProgress,
		justRestarted,
		justFinished,
		finished
	};

	using Duration = std::chrono::microseconds;

	Timer() = default;

	template <class TDuration>
	Timer(TDuration _timer, bool _repeated = false)
	{
		start(_timer, _repeated);
	}

	void start(Duration _timer, bool _repeated = false)
	{
		if (_timer.count() > 0)
		{
			m_Begin = _timer;
			m_Timer = _timer;
			m_Repeated = _repeated;
			m_State = State::inProgress;
		}
		else
			reset();
	}

	template <class TDuration>
	void start(TDuration _timer, bool _repeated = false)
	{
		start(std::chrono::duration_cast<Duration>(_timer), _repeated);
	}

	void restart()
	{
		start(m_Begin, m_Repeated);
	}

	bool isRepeated() const
	{
		return m_Repeated;
	}

	void setRepeated(bool _repeat)
	{
		m_Repeated = _repeat;
	}

	void reset()
	{
		m_Begin = Duration::zero();
		m_Timer = Duration::zero();
		m_State = State::none;
	}

	State getState() const
	{
		return m_State;
	}

	bool isActive() const
	{
		return m_State == State::inProgress ||
			m_State == State::justRestarted;
	}

	Duration getTimeElapsed() const
	{
		return m_Begin - m_Timer;
	}

	Duration getTimerTotal() const
	{
		return m_Begin;
	}

	Duration getTimerValue() const
	{
		return m_Timer;
	}

	std::chrono::milliseconds getTimerValueInMsec() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_Timer);
	}

	float getProgressInPercent() const
	{
		switch (m_State)
		{
		case State::inProgress:
		case State::justRestarted:
			return static_cast<float>(getTimeElapsed().count()) * 100 / getTimerTotal().count();
			break;
		case State::justFinished:
		case State::finished:
			return 100.f;
		}
		return 0;
	}

	template <class TDuration>
	Duration update(TDuration _diff)
	{
		return update(std::chrono::duration_cast<Duration>(_diff));
	}

	Duration update(Duration _diff)
	{
		assert(_diff.count() >= 0);
		if (!isActive() || _diff == Duration::zero())
		{
			if (m_State == State::justFinished)
				m_State = State::finished;
			return Duration::zero();
		}

		Duration used;
		if (m_Timer <= _diff)
		{
			used = m_Timer;
			m_Timer = Duration::zero();
			if (m_Repeated)
			{
				m_State = State::justRestarted;
				m_Timer = m_Begin;
			}
			else
				m_State = State::justFinished;
		}
		else
		{
			m_Timer -= _diff;
			used = _diff;
		}
		return used;
	}

private:
	Duration m_Begin{ Duration::zero() };
	Duration m_Timer{ Duration::zero() };
	bool m_Repeated = false;
	State m_State = State::none;
};
