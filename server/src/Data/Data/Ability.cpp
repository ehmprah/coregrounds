#include "Ability.hpp"
#include "Core/log.hpp"

namespace game::data
{
	std::string_view Ability::getTypeName() const
	{
		return "Ability";
	}
} // namespace game::data