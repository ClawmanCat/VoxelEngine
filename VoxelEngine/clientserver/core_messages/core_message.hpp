#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>


namespace ve {
    namespace core_message_types {
        constexpr std::string_view MSG_SYNC_MTR          = "ve.sync_mtr";
        constexpr std::string_view MSG_IGNORE_THIS       = "ve.ignore_this";
        constexpr std::string_view MSG_COMPOUND          = "ve.compound";
        constexpr std::string_view MSG_ADD_ENTITY        = "ve.ecs.add_entity";
        constexpr std::string_view MSG_DEL_ENTITY        = "ve.ecs.del_entity";
        constexpr std::string_view MSG_SET_COMPONENT     = "ve.ecs.set_component";
        constexpr std::string_view MSG_UNDO_COMPONENT    = "ve.ecs.undo_set_component";
        constexpr std::string_view MSG_DEL_COMPONENT     = "ve.ecs.del_component";
        constexpr std::string_view MSG_PARTIAL_SYNC      = "ve.ecs.partial_sync";
    }


    enum class message_direction : u8 { TO_SERVER = 0b01, TO_CLIENT = 0b10, BIDIRECTIONAL = 0b11 };
    ve_bitwise_enum(message_direction);


    template <typename T> struct core_message {
        using type = T;

        std::string_view name;
        message_direction direction;

        // Note: may be called before the message handler is added to the instance.
        // Note: this method is also called for the remote MTR with the local instance.
        std::function<void(client&, message_type_registry&)> on_registered_client = nullptr;
        std::function<void(server&, message_type_registry&)> on_registered_server = nullptr;

        std::function<void(client&, message_handler&, const type&)> on_received_client = nullptr;
        std::function<void(server&, message_handler&, const type&)> on_received_server = nullptr;
    };
}