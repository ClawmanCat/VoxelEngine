#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>


namespace ve {
    // Note: message type encodes both the type ID of the underlying message as well as the type of the component.
    struct partial_sync_message {
        std::vector<u8> data;
        u64 message_type;
        entt::entity entity;
    };


    template <typename Instance>
    void on_msg_partial_sync_received(Instance& instance, message_handler& handler, const partial_sync_message& msg);


    const inline core_message<partial_sync_message> msg_partial_sync {
        .name               = core_message_types::MSG_PARTIAL_SYNC,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_partial_sync_received<client>,
        .on_received_server = on_msg_partial_sync_received<server>
    };
}