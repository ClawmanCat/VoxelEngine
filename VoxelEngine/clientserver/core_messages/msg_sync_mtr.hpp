#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>



namespace ve {
    struct mtr_friend_access {
        void register_type(message_type_registry& mtr, std::string name, std::size_t type_id, mtr_id id) {
            mtr.register_type(std::move(name), type_id, id);
        }
    };


    struct mtr_sync_message {
        std::string name;
        u64 type_hash;
        mtr_id id;
    };


    template <typename Instance>
    inline void on_msg_sync_mtr_received(Instance& instance, message_handler& handler, const mtr_sync_message& msg) {
        if (!handler.get_remote_mtr().contains(msg.name)) {
            VE_LOG_DEBUG(cat("Registering new MTR type ", msg.name, " on ", instance.get_name(), " for remote ", handler.get_remote_id(), " with ID ", msg.id, "."));
            mtr_friend_access{}.register_type(handler.get_remote_mtr(), msg.name, msg.type_hash, msg.id);
        } else {
            const message_type& type = handler.get_remote_mtr().get_type(msg.name);

            if (type.id == msg.id && type.type_hash == msg.type_hash) {
                VE_LOG_WARN(cat("Ignoring re-registration of already registered type ", type.name));
            } else {
                VE_LOG_ERROR(cat(
                    "Ignoring re-registration of already registered type ", type.name,
                    " which was re-registered with different parameters:\n",
                    "Current ID: ", type.id, ", New ID: ", msg.id, "\n",
                    "Current Type:", type.type_hash, ", New Type: ", msg.type_hash
                ));
            }
        }
    }


    // This message type is used to synchronize other message types between different instances.
    // Note: dispatching of this message is handled automatically by the message handler.
    const inline core_message<mtr_sync_message> msg_sync_mtr {
        .name               = core_message_types::MSG_SYNC_MTR,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_sync_mtr_received<client>,
        .on_received_server = on_msg_sync_mtr_received<server>
    };
}