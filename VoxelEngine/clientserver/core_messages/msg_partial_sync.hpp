#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>

#include <entt/entt.hpp>


namespace ve {
    struct partial_sync_message {
        std::vector<u8> message;
        u64 component_type;
        u64 message_type;
        entt::entity entity;
    };


    template <typename Instance>
    extern void on_msg_partial_sync_received(Instance& i, message_handler& handler, const partial_sync_message& msg);


    // Called when a component performs partial synchronization using the partially_synchronized interface.
    const inline core_message<partial_sync_message> msg_partial_sync {
        .name               = core_message_types::MSG_PARTIAL_SYNC,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_partial_sync_received<client>,
        .on_received_server = on_msg_partial_sync_received<server>
    };
}