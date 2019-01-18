#include "stdafx.h"
#include "GameStatistics.hpp"

namespace game {

	json::Object  Statistics::toJson() const
	{
		assert(!players.empty());
		json::Object  result;
		result["match_type"] = matchType;
		result["tempMatchId"] = tempMatchId;
		result["winner_team"] = !winnerTeam ? 0 : *winnerTeam;
		result["duration"] = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

		json::Object playerArray;
		for (auto& player : players)
		{
			json::Object playerObject;

			struct TypeDataHelper
			{
				json::Object& playerObj;

				void operator ()(const ParticipantData::Human& _data)
				{
					playerObj["uid"] = _data.uid;
				}

				void operator ()(const ParticipantData::Bot& _data)
				{
					playerObj["bot_difficulty"] = _data.difficulty;
				}
			};

			std::visit(TypeDataHelper{ playerObject }, player.typeData);
			playerObject["team_id"] = player.teamId;

			playerObject["minions_spawned"] = player.minionSpawned;
			playerObject["minions_killed"] = player.minionKilled;
			playerObject["walls_built"] = player.wallSpawned;
			playerObject["walls_killed"] = player.wallKilled;
			playerObject["towers_killed"] = player.towerKilled;

			playerObject["damage"] = player.damageAmount;
			playerObject["healing"] = player.healAmount;
			playerObject["core_health"] = player.coreMinHealth;

			if (player.modPageId)
				playerObject["modpage_index"] = *player.modPageId;

			playerObject["factory_bans"] = player.factoryBans;
			playerObject["ability_bans"] = player.abilityBans;

			json::Object factories;
			for (auto& factory : player.factories)
			{
				json::Object factoryObj;
				factoryObj["id"] = factory.id;
				factoryObj["xp"] = factory.xpGained;
				factoryObj["upgrades"] = factory.upgrades;
				factories.push_back(std::move(factoryObj));
			}
			playerObject["factories"] = std::move(factories);

			json::Object abilities;
			for (auto& ability : player.abilities)
			{
				json::Object abilityObj;
				abilityObj["id"] = ability.id;
				abilityObj["uses"] = ability.uses;
				abilities.push_back(std::move(abilityObj));
			}
			playerObject["abilities"] = std::move(abilities);
			playerArray.push_back(std::move(playerObject));
		}
		result["players"] = std::move(playerArray);
		return result;
	}
} // namespace game
