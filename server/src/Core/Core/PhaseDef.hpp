#pragma once

#include <stdexcept>

namespace game::phase
{
	enum class Type
	{
		none,
		pick,
		game,
		finalize
	};

	class Interface;
	class Pick;
	class AbstractGame;
	class Finalize;

	class Exception :
		public std::runtime_error
	{
	private:
		using super = std::runtime_error;

	public:
		Exception(const std::string& _what) :
			super(_what)
		{}
	};
} // namespace game::phase
