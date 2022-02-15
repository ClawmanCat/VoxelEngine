#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>


namespace ve {
    void server::initialize_mtr(void) {
        register_core_messages(*this, get_mtr());
    }
}