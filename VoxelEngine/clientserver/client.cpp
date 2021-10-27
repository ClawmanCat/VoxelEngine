#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>


namespace ve {
    void client::initialize_mtr(void) {
        register_core_messages(*this, mtr);
    }
}