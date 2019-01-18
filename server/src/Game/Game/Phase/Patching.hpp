#pragma once

namespace game
{
	protobuf::out::Message createPatch(const protobuf::out::Message& _old, const protobuf::out::Message& _new);
} // namespace game