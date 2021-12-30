#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/clientserver_communication_common.hpp>
#include <VoxelEngine/clientserver/connect.hpp>


constexpr std::size_t num_clients = 32;


test_result test_main(void) {
    return test_with_connection_method(ve::connect_local, ve::disconnect_local, num_clients);
}