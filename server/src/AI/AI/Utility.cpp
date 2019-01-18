#include "stdafx.h"
#include "Utility.hpp"

bool isBuildable(const game::TilePosition& _pos, const std::string& _map)
{
	assert(MapWidth * MapHeight <= _map.size() * 8);
	auto index = _pos.getY() * MapWidth + _pos.getX();
	auto byte = _map[index / 8];
	return ((byte >> (index % 8)) & 1);
}

std::vector<game::TilePosition> getBuildablePositions(const std::string& _map)
{
	std::vector<game::TilePosition> result;
	for (int y = 0; y < MapHeight; ++y)
	{
		for (int x = 0; x < MapWidth; ++x)
		{
			auto index = y * MapWidth + x;
			auto byte = _map[index / 8];
			if ((byte >> (index % 8)) & 1)
				result.emplace_back(x, y);
		}
	}
	return result;
}

std::optional<game::TileRect> generateBuildableArea(int _playerIndex)
{
	return game::TileRect(_playerIndex == 0 ? 0 : 8, 0, 7, 7);
}

std::uint32_t serializeTilePos(const game::TilePosition& _pos)
{
	std::uint32_t result = 0;
	result |= _pos.getX() * 1000;
	result |= (_pos.getY() * 1000) << 16;
	return result;
}

game::TilePosition deserializeTilePos(std::uint32_t _value)
{
	auto at = removeDirtyFlag(_value);
	auto mask = static_cast<std::uint32_t>(std::pow(2, 15)) - 1;
	game::TilePosition pos(at & mask, (at >> 15) & mask);
	pos /= 1000;
	return pos;
}

game::AbsPosition jitterAbsPos(game::AbsPosition _pos) 
{
	_pos += randomInt(-100, 100) / 1000.0;
	return _pos;
}

std::uint32_t serializeAbsPos(const game::AbsPosition& _pos)
{
	std::uint32_t result = 0;
	result |= static_cast<std::uint32_t>(_pos.getX() * 1000);
	result |= static_cast<std::uint32_t>(_pos.getY() * 1000) << 16;
	return result;
}

game::AbsPosition deserializeAbsPos(std::uint32_t _value)
{
	auto at = removeDirtyFlag(_value);
	auto mask = static_cast<std::uint32_t>(std::pow(2, 15)) - 1;
	game::AbsPosition pos(at & mask, (at >> 15) & mask);
	pos /= 1000;
	return pos;
}

std::chrono::seconds getTimeInSeconds(std::uint32_t _value)
{
	auto cd = removeDirtyFlag(_value);
	return std::chrono::seconds((cd & 32767) / 10);
}

std::chrono::milliseconds getTimeInMSec(std::uint32_t _value)
{
	auto cd = removeDirtyFlag(_value);
	return std::chrono::milliseconds((cd & 32767) * 100);
}

int getTotalUpgrades(std::uint32_t _value)
{
	auto clean = removeDirtyFlag(_value);
	return (clean & 7) + ((clean >> 3) & 7) + (clean >> 6);
}

int getCurrentStacks(std::uint32_t _value)
{
	auto clean = removeDirtyFlag(_value);
	return clean & 127;
}

int getMaxStacks(std::uint32_t _value)
{
	auto clean = removeDirtyFlag(_value);
	return (clean >> 7) & 127;
}

int getUpgradesByIndex(std::uint32_t _value, size_t _index)
{
	auto clean = removeDirtyFlag(_value);
	if (0 == _index) return clean & 7;
	if (1 == _index) return (clean >> 3) & 7;
	if (2 == _index) return (clean >> 6);
	return 0;
}

const protobuf::out::Match::Game::Player& getPlayer(const protobuf::out::Match_Game& _output)
{
	auto playerIndex = removeDirtyFlag(_output.playerindex());
	auto& opponent = _output.player(playerIndex);
	return opponent;
}

const protobuf::out::Match::Game::Player& getOpponent(const protobuf::out::Match_Game& _output)
{
	auto playerIndex = 1 - removeDirtyFlag(_output.playerindex());
	auto& opponent = _output.player(playerIndex);
	return opponent;
}

game::AbsPosition getPosition(const protobuf::out::Match::Game::Unit & _unit)
{
	return deserializeAbsPos(_unit.position());
}

protobuf::in::Match::Game::ActivateFactory createFactoryActivation(int _index)
{
	protobuf::in::Match::Game::ActivateFactory activation;
	activation.set_index(_index);
	return activation;
}

protobuf::in::Match::Game::ActivateFactory createFactoryActivation(int _index, game::TilePosition _pos)
{
	protobuf::in::Match::Game::ActivateFactory activation;
	activation.set_index(_index);
	activation.set_position(serializeTilePos(_pos));
	return activation;
}

protobuf::in::Match::Game::UpgradeFactory createFactoryUpgrade(int _factoryIndex, int upgradeIndex)
{
	protobuf::in::Match::Game::UpgradeFactory activation;
	activation.set_index(_factoryIndex);
	activation.set_upgrade(upgradeIndex);
	return activation;
}