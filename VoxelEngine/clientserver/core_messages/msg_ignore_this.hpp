#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/utility/io/serialize/overloadable_serializer.hpp>


namespace ve {
    struct null_message {
        void to_bytes(std::vector<u8>&) const {}
        static null_message from_bytes(std::span<const u8>&) { return null_message {}; }
    };


    // Messages of this type are ignored by the rest of the engine and can be used to send data outside the messaging system.
    const inline core_message<null_message> msg_ignore_this {
        .name      = core_message_types::MSG_IGNORE_THIS,
        .direction = message_direction::BIDIRECTIONAL
    };
}