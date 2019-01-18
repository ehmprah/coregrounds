#include "Core/AI.hpp"

#ifdef AI_EXPORTS
#define AI_API __declspec(dllexport)
#else
#define AI_API __declspec(dllimport)
#endif

namespace game {
namespace ai {

	class AI_API Creator
	{
	public:
		static Ptr loadAI(const Initializer& _initializer);
	};
} // namespace ai
} // namespace game
