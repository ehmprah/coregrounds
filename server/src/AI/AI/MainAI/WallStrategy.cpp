#include "stdafx.h"
#include "WallStrategy.hpp"

namespace game::ai 
{

	LayoutWalls::LayoutWalls(const data::AiMazeLayout& _layout) :
		m_Layout(_layout)
	{
	}

	const StrategyResponse LayoutWalls::update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output)
	{
		StrategyResponse response;

		int index = 0;
		auto playerIndex = removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);
		for (auto& factory : player.factories())
		{
			if (factory.has_wall())
			{
				if (getCurrentStacks(factory.wall().stacks()) > 0)
				{
					if (auto itr = std::find_if(
						std::begin(m_Layout.positions),
						std::end(m_Layout.positions),
						[&buildingMap = player.buildingmap()](const auto& _pos) { return isBuildable(_pos, buildingMap); }
					); itr != std::end(m_Layout.positions))
					{
						auto& activation = *_input.add_factoryactivations();
						activation.set_position(serializeTilePos(*itr));
						activation.set_index(index);
						response.shouldReturn = true;
						return response;
					}

					// If we didn't find a layout target, we build at random to fill the map
					auto positions = getBuildablePositions(player.buildingmap());
					if (!positions.empty())
					{
						auto pos = getRandomElement(positions);
						auto& activation = *_input.add_factoryactivations();
						activation.set_position(serializeTilePos(pos));
						activation.set_index(index);
						response.shouldReturn = true;
						return response;
					}
				}
			}
			++index;
		}
		return response;
	}

	const StrategyResponse RandomWalls::update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output)
	{
		StrategyResponse response;
		int index = 0;
		auto playerIndex = removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);
		for (auto& factory : player.factories())
		{
			if (factory.has_wall())
			{
				if (factory.wall().stacks() > 0)
				{
					auto positions = getBuildablePositions(player.buildingmap());
					auto pos = getRandomElement(positions);
					auto& activation = *_input.add_factoryactivations();
					activation.set_position(serializeTilePos(pos));
					activation.set_index(index);
					response.shouldReturn = true;
					return response;
				}
			}
			++index;
		}
		return response;
	}
}