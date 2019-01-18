#include "stdafx.h"
#include "AICreator.hpp"
#include "MainAI\MainAI.hpp"
#include "EmptyAI.hpp"
#include "ExampleAI.hpp"

namespace game::ai
{
	Ptr Creator::loadAI(const Initializer& _initializer)
	{
#ifndef NDEBUG
		auto& debug = _initializer.gameData.gameProperties.game.debug;
		if (debug && *debug->emptyAI)
			return std::make_unique<EmptyAI>(_initializer);
#endif
		return std::make_unique<MainAI>(_initializer);
	}
} // namespace game::ai
