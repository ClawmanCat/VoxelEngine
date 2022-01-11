#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>


namespace ve {
    struct del_component_message {
        u64 component_type;
        entt::entity entity;
    };


    template <typename Instance>
    inline void on_msg_del_component_received(Instance& instance, message_handler& handler, const del_component_message& msg) {
        const auto& component_data = component_registry::instance().get(msg.component_type);
        component_data.remove_component(instance, msg.entity);
    }


    // Message to add a set of entities to the registry.
    const inline core_message<del_component_message> msg_del_component {
        .name               = core_message_types::MSG_DEL_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_del_component_received<client>,
        .on_received_server = on_msg_del_component_received<server>
    };
}