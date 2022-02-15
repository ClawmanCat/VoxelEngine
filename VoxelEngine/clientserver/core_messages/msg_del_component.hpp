#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>


namespace ve {
    struct del_component_message {
        u64 component_type;
        entt::entity entity;
    };


    template <typename Instance>
    inline void on_msg_del_component_received(Instance& instance, message_handler& handler, const del_component_message& msg) {
        const auto& component_data = component_registry::instance().get(msg.component_type);

        auto [allowed, value] = component_data.remove_component_checked(
            handler.get_remote_id(),
            instance,
            msg.entity
        );

        if (allowed == change_result::FORBIDDEN) {
            handler.send_message(
                msg_undo_set_component.name,
                undo_component_message { std::move(value), msg.component_type, msg.entity }
            );
        }
    }


    // Message to add a set of entities to the registry.
    const inline core_message<del_component_message> msg_del_component {
        .name               = core_message_types::MSG_DEL_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_del_component_received<client>,
        .on_received_server = on_msg_del_component_received<server>
    };
}