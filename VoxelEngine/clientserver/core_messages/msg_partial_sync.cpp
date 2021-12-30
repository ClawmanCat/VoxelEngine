#include <VoxelEngine/clientserver/core_messages/msg_partial_sync.hpp>
#include <VoxelEngine/ecs/partially_synchronized.hpp>


namespace ve {
    template <typename Instance>
    inline void on_msg_partial_sync_received(Instance& i, message_handler& handler, const partial_sync_message& msg) {
        detail::partially_synchronized_type_registry::instance().invoke_callback(
            msg.component_type,
            msg.message_type,
            i,
            msg.entity,
            handler.get_remote_id(),
            std::span<const u8> { msg.message.begin(), msg.message.end() }
        );
    }


    template void on_msg_partial_sync_received<client>(client& i, message_handler& handler, const partial_sync_message& msg);
    template void on_msg_partial_sync_received<server>(server& i, message_handler& handler, const partial_sync_message& msg);
}