#include <VoxelEngine/clientserver/core_messages/msg_partial_sync.hpp>
#include <VoxelEngine/ecs/component/partially_synchronizable.hpp>


namespace ve {
    template <typename Instance>
    void on_msg_partial_sync_received(Instance& instance, message_handler& handler, const partial_sync_message& msg) {
        detail::ps_message_registry::instance().get(msg.message_type).invoke_on_received(
            instance,
            msg.entity,
            msg.data,
            handler.get_remote_id()
        );
    }


    template void on_msg_partial_sync_received<client>(client& instance, message_handler& handler, const partial_sync_message& msg);
    template void on_msg_partial_sync_received<server>(server& instance, message_handler& handler, const partial_sync_message& msg);
}