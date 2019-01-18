#pragma once

#include "AI\Utility.hpp"
#include "StrategyResponse.hpp"

namespace game::ai 
{

	class WallStrategy
	{
		public:
			virtual ~WallStrategy() = default;
			virtual const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) = 0;
	};

	class LayoutWalls : public WallStrategy
	{
		public:
			LayoutWalls(const data::AiMazeLayout& _layout);
			const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) override;

		private:
			const data::AiMazeLayout& m_Layout;
	};

	class RandomWalls : public WallStrategy
	{
	public:
		RandomWalls() {}

		const StrategyResponse update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output) override;
	};
}