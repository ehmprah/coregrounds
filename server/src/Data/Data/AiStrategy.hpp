#pragma once

#include "Core/SharedDefines.hpp"
#include "PropertyDefines.hpp"

namespace game::data
{
	struct AiMazeLayout
	{
		prop::Property<ID> id{ prop::Id };
		prop::Property<std::string> name{ prop::Name };
		std::vector<TilePosition> positions;
	};

	struct AiStrategy
	{
		prop::Property<ID> id{ prop::Id };
		prop::Property<std::string> name{ prop::Name };
		IDs mazeLayouts;

		prop::Property<int> botLevelMin{ "botlevel_min", 0, prop::GreaterEqualZero };
		prop::Property<int> botLevelMax{ "botlevel_max", 0, prop::GreaterEqualZero };

		std::vector<std::string> flags;

		std::vector<ModPage> modPages;
		std::vector<IDs> factoryPicks;
		std::vector<IDs> abilityPicks;
		std::vector<IDs> factoryBans;
		std::vector<IDs> abilityBans;
		IDs skins;
	};
} // namespace game::data
