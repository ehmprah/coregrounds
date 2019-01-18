#include "stdafx.h"
#include "AbilityStrategy.hpp"
#include "HeatMap.hpp"

namespace game::ai 
{
	protobuf::in::Match::Game::ActivateAbility createAbilityActivation(int _index, const AbsPosition& _pos)
	{
		protobuf::in::Match::Game::ActivateAbility activation;
		activation.set_index(_index);
		activation.set_position(serializeAbsPos(_pos));
		return activation;
	}

	protobuf::in::Match::Game::ActivateAbility createAbilityActivation(int _index, const protobuf::out::Match::Game::Unit & _unit)
	{
		return createAbilityActivation(_index, deserializeAbsPos(_unit.position()));
	}

	AbilityDefault::AbilityDefault(int _botlevel, const StrategyFlags& _flags, const HeatMap& _heatmap) :
		m_BotLevel(_botlevel),
		m_Flags(_flags),
		m_HeatMap(_heatmap)
	{
	}

	StrategyResponse AbilityDefault::update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output)
	{	
		StrategyResponse response;

		// We wait until the end of the downtime
		if (m_Downtime > _output.gametime())
			return response;

		auto playerIndex = removeDirtyFlag(_output.playerindex());
		auto& player = _output.player(playerIndex);
		for (int index = 0; index < std::size(player.abilities()); ++index)
		{
			auto& ability = player.abilities(index);
			if (std::chrono::milliseconds(100) <= getTimeInMSec(ability.cooldown()))
			{
				auto& subStrategy = m_SubStrategies[index];
				if (!subStrategy)
					subStrategy = makeAbilitySubStrategy(removeDirtyFlag(ability.id()), m_BotLevel, m_Flags, m_HeatMap);

				if (auto optPos = subStrategy->update(_output))
				{
					*_input.add_abilityactivations() = createAbilityActivation(index, *optPos);
					m_Downtime = _output.gametime() + 1000;
					response.shouldReturn = true;
					return response;
				}
			}
		}

		return response;
	}
}