#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>


namespace ve {
    struct set_component_message {
        std::vector<u8> component_data;
        u64 component_type;
        entt::entity entity;
    };


    template <typename Instance>
    inline void on_msg_set_component_received(Instance& instance, message_handler& handler, const set_component_message& msg) {
        const auto& component_data = component_registry::instance().get(msg.component_type);
        component_data.set_component(instance, msg.entity, std::span { msg.component_data.begin(), msg.component_data.end() });
    }


    // Message to add a set of entities to the registry.
    const inline core_message<set_component_message> msg_set_component {
        .name               = core_message_types::MSG_SET_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_set_component_received<client>,
        .on_received_server = on_msg_set_component_received<server>
    };
}