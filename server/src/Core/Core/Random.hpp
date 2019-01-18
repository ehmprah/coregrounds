#pragma once

#include "Core/StdInclude.hpp"
#include "Core/SLInclude.hpp"

class Random :
	sl2::NonCopyable
{
private:
	std::ranlux24_base m_Engine;

public:
	Random() :
		m_Engine(std::random_device()())
	{}

	template <class IntType>
	IntType rndInt(IntType _min, IntType _max)
	{
		assert(_min <= _max);
		std::uniform_int_distribution<IntType> dist(_min, _max);
		return dist(m_Engine);
	}
};

template <class IntType = int>
IntType randomInt(IntType _min, IntType _max)
{
	thread_local Random rnd;
	return rnd.rndInt<IntType>(_min, _max);
}

inline bool randomBool()
{
	return randomInt<int>(0, 1) == 1;
}
