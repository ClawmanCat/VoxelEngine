#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/change_validator.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>

#include <magic_enum.hpp>


namespace ve {
    template <bool Rebroadcast> struct set_component_message_tmpl {
        std::vector<u8> component_data;
        u64 component_type;
        entt::entity entity;
    };

    using set_component_message  = set_component_message_tmpl<false>;
    using undo_component_message = set_component_message_tmpl<true>;


    template <typename Instance, bool Rebroadcast>
    inline void on_msg_set_component_received(Instance& instance, message_handler& handler, const set_component_message_tmpl<Rebroadcast>& msg) {
        const auto& component_data = component_registry::instance().get(msg.component_type);

        auto [allowed, value] = component_data.set_component_checked(
            handler.get_remote_id(),
            instance,
            msg.entity,
            std::span { msg.component_data.begin(), msg.component_data.end() }
        );

        // If the change was not allowed, but we can observe the entity, send back the fact that the component is unchanged.
        if (!Rebroadcast && allowed == change_result::FORBIDDEN) {
            handler.send_message(
                core_message_types::MSG_UNDO_COMPONENT,
                undo_component_message { std::move(value), msg.component_type, msg.entity }
            );
        }
    }


    // Message to set the value of a component for some entity.
    const inline core_message<set_component_message> msg_set_component {
        .name               = core_message_types::MSG_SET_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_set_component_received<client, false>,
        .on_received_server = on_msg_set_component_received<server, false>
    };

    // Message sent back if the above message was not executed because the change is not allowed.
    const inline core_message<undo_component_message> msg_undo_set_component {
        .name               = core_message_types::MSG_UNDO_COMPONENT,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_set_component_received<client, true>,
        .on_received_server = on_msg_set_component_received<server, true>
    };
}