#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_sync_mtr.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>


namespace ve {
    template <typename Instance>
    void message_handler::constructor_impl(Instance& instance) {
        register_core_messages(instance, remote_mtr);
        register_core_message_handlers(instance, *this);

        for (const auto& [id, type] : local_mtr->get_types_by_id()) {
            unregistered_local_types.insert(id);
        }

        local_mtr->add_handler([this] (const message_type_registered_event& e) {
            if (!e.type->is_core) {
                register_message_type(e.type->name, e.type->type_hash);
            }
        });
    }

    template void message_handler::constructor_impl<client>(client& instance);
    template void message_handler::constructor_impl<server>(server& instance);


    void message_handler::register_message_type(std::string_view type_name, std::size_t type_hash) {
        const message_type* type = nullptr;

        // If the type has not yet been registered locally, do so.
        const auto& types_by_name = local_mtr->get_types_by_name();
        if (auto it = types_by_name.find(type_name); it != types_by_name.end()) {
            type = &it->second;

            VE_ASSERT(
                type->type_hash == type_hash,
                "Attempt to re-register message type ", type_name,
                " with different underlying data type."
            );

            // Never synchronize core types; the remote already knows them.
            if (type->is_core) return;

            // If the type is registered locally and already synced, there's nothing to do.
            if (auto ult_it = unregistered_local_types.find(type->id); ult_it == unregistered_local_types.end()) return;
            else unregistered_local_types.erase(ult_it);
        } else {
            type = &local_mtr->register_type(std::string { type_name }, type_hash);
        }

        send_message(
            core_message_types::MSG_SYNC_MTR,
            mtr_sync_message { .name = type->name, .type_hash = type->type_hash, .id = type->id }
        );
    }


    void message_handler::on_message_received(const message_t& msg) {
        if (use_queue) [[unlikely]] {
            read_queue.push_back(msg);
            return;
        }


        std::span<const u8> span { msg.begin(), msg.end() };

        mtr_id message_type = serialize::from_bytes<mtr_id>(span);
        const auto& type_info = remote_mtr.get_type(message_type);

        if (auto it = handlers.find(type_info.name); it != handlers.end()) {
            it->second->handle(span);
        }
    }


    void message_handler::toggle_queue(bool enabled) {
        use_queue = enabled;

        if (!use_queue) {
            for (const auto& msg : read_queue) on_message_received(msg);
            read_queue.clear();

            for (const auto& msg : write_queue) send_message(msg);
            write_queue.clear();
        }
    }
}