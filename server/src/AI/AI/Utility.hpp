#pragma once

inline constexpr std::size_t MapWidth = 15;
inline constexpr std::size_t MapHeight = 7;

std::vector<game::TilePosition> getBuildablePositions(const std::string& _map);

bool isBuildable(const game::TilePosition& _pos, const std::string& _map);

std::uint32_t serializeTilePos(const game::TilePosition& _pos);
game::TilePosition deserializeTilePos(std::uint32_t _value);

game::AbsPosition jitterAbsPos(game::AbsPosition _pos);
std::uint32_t serializeAbsPos(const game::AbsPosition& _pos);
game::AbsPosition deserializeAbsPos(std::uint32_t _value);

template <class T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
T removeDirtyFlag(const T& _value)
{
	auto mask = static_cast<T>(std::pow(2, sizeof(_value) * 8 - 1)) - 1;
	return _value & mask;
}

template <class Container>
decltype(auto) getRandomElement(Container& _container)
{
	auto itr = std::begin(_container);
	auto diff = std::distance(itr, std::end(_container));
	assert(0 < diff);
	auto rnd = randomInt<decltype(diff)>(0, diff - 1);
	assert(0 <= rnd && rnd < diff);
	std::advance(itr, rnd);
	assert(itr != std::end(_container));
	return *itr;
}

std::chrono::seconds getTimeInSeconds(std::uint32_t _value);
std::chrono::milliseconds getTimeInMSec(std::uint32_t _value);
int getTotalUpgrades(std::uint32_t _value);
int getUpgradesByIndex(std::uint32_t _value, size_t _index);
int getMaxStacks(std::uint32_t _value);
int getCurrentStacks(std::uint32_t _value);

const protobuf::out::Match::Game::Player& getPlayer(const protobuf::out::Match_Game& _output);
const protobuf::out::Match::Game::Player& getOpponent(const protobuf::out::Match_Game& _output);
game::AbsPosition getPosition(const protobuf::out::Match::Game::Unit & _unit);

protobuf::in::Match::Game::ActivateFactory createFactoryActivation(int _index);
protobuf::in::Match::Game::ActivateFactory createFactoryActivation(int _index, game::TilePosition _pos);
protobuf::in::Match::Game::UpgradeFactory createFactoryUpgrade(int _factoryIndex, int upgradeIndex);