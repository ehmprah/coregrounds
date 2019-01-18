#pragma once

#include "VisualCollector.hpp"

namespace game
{
	struct Statistics;
} // namespace game
namespace game::phase
{
	struct PickState
	{
	};

	struct GameState
	{
	};

	struct FinalizeState
	{
		const Statistics* statistics;
		std::optional<ID>* permMatchId;	// this works like a request. The match handler has to provide some data here.
	};

	using Data = std::variant<std::monostate, PickState, GameState, FinalizeState>;

	class Interface :
		sl2::NonCopyable
	{
	public:
		virtual ~Interface() = default;

		virtual void start() = 0;
		virtual void finalize() = 0;
		virtual Data update(Time _diff) = 0;

		virtual bool hasFinished() const = 0;
		virtual Type getType() const = 0;

		virtual VisualCollector& getVisualCollector() = 0;
		virtual const VisualCollector& getVisualCollector() const = 0;
	};
} // namespace game::phase