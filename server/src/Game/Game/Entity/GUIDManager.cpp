#include "stdafx.h"
#include "GUIDManager.hpp"

namespace game::unit
{
	GUID GUIDManager::getNextGUID()
	{
		if (m_NextGUID == std::numeric_limits<GUID>::max())
			throw std::runtime_error("GuidManager: GUID overflow");
		return m_NextGUID++;
	}
} // namespace game::unit
