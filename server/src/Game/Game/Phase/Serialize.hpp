#pragma once

namespace game
{
	protobuf::out::Message serializeToProtobuf(const phase::Pick& _phase, std::optional<int> _playerIndex);
	protobuf::out::Message serializeToProtobuf(const phase::AbstractGame& _phase, std::optional<int> _playerIndex);
	protobuf::out::Message serializeToProtobuf(const phase::Finalize& _phase, std::optional<int> _playerIndex);
} // namespace game
