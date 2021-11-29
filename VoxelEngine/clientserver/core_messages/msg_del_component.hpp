#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_set_component.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>

#include <entt/entt.hpp>


namespace ve {
    struct del_component_message {
        u64 component_type;
        entt::entity entity;
    };


    template <typename Instance>
    inline void on_msg_del_component_received(Instance& instance, message_handler& handler, const del_component_message& msg) {
        auto data   = component_registry::instance().get_component_data(msg.component_type);
        auto result = data.remove_component(handler.get_remote_id(), instance, msg.entity);

        if (result == change_result::DENIED) {
            // The change was denied but the remote is allowed to observe that is was denied,
            // so send back the old value of the component.
            handler.send_message(
                core_message_types::MSG_SET_COMPONENT,
                set_component_message {
                    .data           = data.get_component(instance, msg.entity),
                    .component_type = msg.component_type,
                    .entity         = msg.entity
                }
            );
        }
    }


    // Called when a component is removed from the ECS.
    const inline core_message<del_component_message> msg_del_component {
        .name               = core_message_types::MSG_DEL_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_del_component_received<client>,
        .on_received_server = on_msg_del_component_received<server>
    };
}