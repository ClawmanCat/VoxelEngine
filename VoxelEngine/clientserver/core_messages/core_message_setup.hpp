#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/tuple_foreach.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/traits/is_immovable.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <VoxelEngine/clientserver/core_messages/msg_sync_mtr.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_ignore_this.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>


namespace ve {
    const inline std::tuple core_message_type_infos {
        msg_sync_mtr,
        msg_ignore_this,
        msg_compound
    };


    inline mtr_id get_core_mtr_id(std::string_view name) {
        mtr_id result = 0;

        tuple_foreach(core_message_type_infos, [&] (const auto& info) {
            if (info.name == name) return false;

            ++result;
            return true;
        });

        VE_DEBUG_ASSERT(result < std::tuple_size_v<decltype(core_message_type_infos)>, "No such core message:", name);
        return result;
    }


    // Register the existence of all core message types into the provided MTR.
    // This should be done when the instance is constructed.
    template <typename Instance> requires (meta::pack<client, server>::template contains<Instance>)
    inline void register_core_messages(Instance& owner, message_type_registry& mtr) {
        auto direction = std::is_same_v<Instance, client>
             ? message_direction::TO_SERVER
             : message_direction::TO_CLIENT;

        // Bypass of message_handler::register_message_type is intentional:
        // core message types are not synchronized, as they will be the same on both sides already anyway.
        tuple_foreach(
            core_message_type_infos,
            [&] <typename Info> (const Info& info) {
                mtr.template register_type<typename Info::type>(std::string { info.name }, true);

                const auto& fn = meta::pick<std::is_same_v<Instance, client>>(
                    info.on_registered_client,
                    info.on_registered_server
                );

                if (fn && (info.direction & direction)) fn(owner, mtr);
            }
        );
    }


    // Register required handlers to be called when a core message is received in the provided message handler.
    // This should be done when the message handler is constructed.
    template <typename Instance> requires (meta::pack<client, server>::template contains<Instance>)
    inline void register_core_message_handlers(Instance& owner, message_handler& handler) {
        static_assert(meta::is_immovable_v<Instance>, "Instances must be immovable so their address may be safely stored.");
        static_assert(meta::is_immovable_v<message_handler>, "Message handlers must be immovable so their address may be safely stored.");

        // The handler needs to be added to the opposite side of the one that's sending the message.
        auto direction = std::is_same_v<Instance, client>
            ? message_direction::TO_CLIENT
            : message_direction::TO_SERVER;

        tuple_foreach(
            core_message_type_infos,
            [&] <typename Info> (const Info& info) {
                const auto& fn = meta::pick<std::is_same_v<Instance, client>>(
                    info.on_received_client,
                    info.on_received_server
                );

                if (fn && (info.direction & direction)) {
                    handler.add_handler(info.name, [&] (const typename Info::type& msg) {
                        fn(owner, handler, msg);
                    });
                }
            }
        );
    }
}