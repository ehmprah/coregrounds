#pragma once

#include "Core/SLInclude.hpp"
#include "Core/SharedDefines.hpp"
#include "Core/protobuf/Includes.hpp"
#include "Core/ParticipantData.hpp"

namespace game
{
namespace data
{
	class Game;
} // namespace data
namespace ai
{
	struct Initializer
	{
		const data::Game& gameData;
		int botLevel;
	};

	class Interface :
		sl2::NonCopyable
	{
	public:
		virtual ~Interface() = default;

		virtual const ParticipantData& getParticipantData() const = 0;
		virtual int getBotLevel() const = 0;
		virtual const data::Game& getGameData() const = 0;
		virtual std::optional<protobuf::in::Match::Game> update(Time _diff, const protobuf::out::Match::Game& _msg) = 0;
		virtual std::optional<protobuf::in::Match::Pick> update(Time _diff, const protobuf::out::Match::Pick& _msg) = 0;
		virtual std::optional<protobuf::in::Match::Finalize> update(Time _diff, const protobuf::out::Match::Finalize& _msg) = 0;
	};
	using Ptr = std::unique_ptr<Interface>;
} // namespace ai
} // namespace game
