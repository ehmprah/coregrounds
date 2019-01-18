#pragma once

namespace game::unit
{
	class GUIDManager :
		sl2::NonCopyable
	{
	private:
		GUID m_NextGUID = 1;

	public:
		GUID getNextGUID();
	};
} // namespace game::unit
