#pragma once

#include "Shape.hpp"
#include "PropertyDefines.hpp"
#include "Core/log.hpp"

BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(DataLogger, ChannelLogger, (boost::log::keywords::channel = "data"));
inline ChannelLogger& dataLogger{ DataLogger::get() };

namespace game::data
{
	class Error : public std::runtime_error
	{
	private:
		using super = std::runtime_error;

	public:
		Error(const std::string& _what);
	};

	struct Color
	{
		prop::Property<ID> id{ prop::Id };
	};

	struct PlayerIcon
	{
		prop::Property<ID> id{ prop::Id };
	};

	struct Skin
	{
		prop::Property<ID> id{ prop::Id };
	};

	struct Background
	{
		prop::Property<ID> id{ prop::Id };
	};

	struct Emote
	{
		prop::Property<ID> id{ prop::Id };
		prop::Property<std::string> emote{ "emote", "" };
	};

	struct UpgradeDuration
	{
		float time = 0;
	};

	struct Modification
	{
		prop::Property<ID> id{ prop::Id };
		prop::Property<std::string> name{ prop::Name };
		prop::Property<ID> spellId{ prop::SpellId };
	};

	struct Unlock
	{
		prop::Property<ID> id{ prop::Id };
		using Type = UnlockType;
		Type typeId;
	};

	template <class Type>
	struct XpBoostModifier
	{
		std::array<Stat, static_cast<std::size_t>(Type::max) - 1> modifiers{};

		Stat getModifier(Type _type) const
		{
			if (_type <= Type::none || _type >= Type::max)
				return 0;
			return modifiers[static_cast<std::size_t>(_type) - 1];
		}
	};
} // namespace game::data
