#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>

#include <entt/entt.hpp>


namespace ve {
    struct add_del_entity_message {
        std::vector<entt::entity> entities;
    };


    template <bool Add>
    inline void on_msg_add_del_entity_received(client& c, message_handler& handler, const add_del_entity_message& msg) {
        for (const auto& entity : msg.entities) {
            if constexpr (Add) c.create_entity_with_id(entity);
            else c.destroy_entity(entity);
        }
    }


    // Called when an entity is added or removed from the ECS.
    const inline core_message<add_del_entity_message> msg_add_entity {
        .name               = core_message_types::MSG_ADD_ENTITY,
        .direction          = message_direction::TO_CLIENT,
        .on_received_client = on_msg_add_del_entity_received<true>
    };

    const inline core_message<add_del_entity_message> msg_del_entity {
        .name               = core_message_types::MSG_DEL_ENTITY,
        .direction          = message_direction::TO_CLIENT,
        .on_received_client = on_msg_add_del_entity_received<false>
    };
}