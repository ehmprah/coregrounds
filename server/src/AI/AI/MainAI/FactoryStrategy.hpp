#pragma once

#include "AI\stdafx.h"
#include "AI\Utility.hpp"

namespace game::ai {

	class FactoryStrategy
	{
	public:
		virtual ~FactoryStrategy() = default;

		virtual bool update(protobuf::in::Match_Game& _input, const protobuf::out::Match_Game& _output, const protobuf::out::Match_Game_Factory& _factory, const int _index) = 0;
	};
}