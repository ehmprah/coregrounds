#include "stdafx.h"
#include "Defines.hpp"

namespace network
{
	Error::Error(std::string_view _what) :
		std::runtime_error(_what.data())
	{
	}
} // namespace network
