#pragma once

#include "Core/EntityFwd.hpp"
#include "Core/SharedDefines.hpp"

namespace game
{
	struct TargetPreference :
		sl2::operators::Equal<TargetPreference>
	{
		using Mode = unit::targetPreference::Mode;
		using Target = unit::targetPreference::Target;
		using Flag = unit::targetPreference::Flag;

		Mode mode = Mode::low;
		sl2::Bitmask<Target> target{ Target::all };
		sl2::Bitmask<Flag> flags;

		friend bool operator ==(const TargetPreference& _lhs, const TargetPreference& _rhs)
		{
			return _lhs.target == _rhs.target &&
				_lhs.mode == _rhs.mode &&
				_lhs.flags == _rhs.flags;
		}
	};
} // namespace game
