#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/utility/io/serialize/overloadable_serializer.hpp>


namespace ve {
    class message_type_registry;


    struct null_message {
        void to_bytes(std::vector<u8>&) const {}
        static null_message from_bytes(std::span<const u8>&) { return null_message {}; }
    };


    // Messages of this type are ignored by the rest of the engine and can be used to send data outside the messaging system.
    const inline core_message<null_message> msg_ignore_this {
        .name      = core_message_types::MSG_IGNORE_THIS,
        .direction = message_direction::BIDIRECTIONAL
    };


    // Modify the given message so that it is ignored by the message handling system.
    extern void make_message_ignored(const message_type_registry& mtr, std::vector<u8>& msg);
    // Modify a message modified with make_message_ignored to get the original message back.
    extern std::span<const u8> make_message_unignored(std::span<const u8> msg);
}