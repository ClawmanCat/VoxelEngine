#include <VoxelEngine/clientserver/core_messages/msg_ignore_this.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    // Modify the given message so that it is ignored by the message handling system.
    void make_message_ignored(const message_type_registry& mtr, std::vector<u8>& msg) {
        mtr_id id = mtr.get_type(core_message_types::MSG_IGNORE_THIS).id;
        serialize::to_bytes(id, msg);
    }


    // Modify a message modified with make_message_ignored to get the original message back.
    std::span<const u8> make_message_unignored(std::span<const u8> msg) {
        return msg.first(msg.size() - sizeof(mtr_id));
    }
}