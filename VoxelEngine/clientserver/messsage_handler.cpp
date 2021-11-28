#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_sync_mtr.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>


namespace ve {
    void message_handler::register_message_type_local(std::string_view type, u64 type_hash) {
        // Core messages need not be checked for separately, as they will always show up as registered.
        if (!local_mtr->contains(type)) [[unlikely]] {
            local_mtr->register_type(std::string { type }, type_hash);
        }
    }


    void message_handler::register_message_type_remote(mtr_id type, u64 type_hash) {
        // Type should already be registered locally.
        const auto& type_data = local_mtr->get_type(type);


        if (!published_types.contains(type_data.id)) [[unlikely]] {
            // Make sure this isn't a core message type, as we don't have to sync those.
            if (type_data.is_core) return;

            // Otherwise send it to the remote.
            send_message(
                core_message_types::MSG_SYNC_MTR,
                mtr_sync_message { .name = type_data.name, .type_hash = type_data.type_hash, .id = type_data.id }
            );

            published_types.insert(type_data.id);
        }
    }


    template <typename Instance> void message_handler::init(Instance& instance) {
        register_core_messages(instance, remote_mtr);
        register_core_message_handlers(instance, *this);
    }

    template void message_handler::init<client>(client& instance);
    template void message_handler::init<server>(server& instance);
}