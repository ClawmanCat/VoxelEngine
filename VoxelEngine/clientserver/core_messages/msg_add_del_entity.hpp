#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>


namespace ve {
    struct add_del_entity_message {
        std::vector<entt::entity> changed;
    };


    template <bool Add>
    inline void on_msg_add_del_entity_received(client& instance, message_handler& handler, const add_del_entity_message& msg) {
        for (auto entity : msg.changed) {
            if constexpr (Add) instance.create_entity_with_id(entity);
            else instance.destroy_entity(entity);
        }
    }


    // Message to add a set of entities to the registry.
    const inline core_message<add_del_entity_message> msg_add_entity {
        .name               = core_message_types::MSG_ADD_ENTITY,
        .direction          = message_direction::TO_CLIENT,
        .on_received_client = on_msg_add_del_entity_received<true>
    };

    // Message to remove a set of entities from the registry.
    const inline core_message<add_del_entity_message> msg_del_entity {
        .name               = core_message_types::MSG_DEL_ENTITY,
        .direction          = message_direction::TO_CLIENT,
        .on_received_client = on_msg_add_del_entity_received<false>
    };
}