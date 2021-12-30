#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/clientserver/socket/socket_client.hpp>
#include <VoxelEngine/clientserver/socket/socket_server.hpp>


constexpr std::size_t num_clients = 32;
const std::vector<ve::u8> message = std::vector<ve::u8> { 0xAB, 0xCD, 0xEF };


test_result test_main(void) {
    auto server = ve::connection::socket_server::create(32);
    server->start(12000);

    std::vector<ve::shared<ve::connection::socket_client>> clients;
    for (std::size_t i = 0; i < num_clients; ++i) {
        auto& client = clients.emplace_back(ve::connection::socket_client::create());
        client->start("127.0.0.1", 12000);
    }


    ve::u32 client_received = 0, server_received = 0;
    test_result result = VE_TEST_SUCCESS;

    server->add_handler([&] (const ve::connection::message_received_event& e) {
        if (*e.message != message) result |= VE_TEST_FAIL("Received message contained incorrect data.");
        ++server_received;
    });

    for (auto& client : clients) {
        client->add_handler([&] (const ve::connection::message_received_event& e) {
            if (*e.message != message) result |= VE_TEST_FAIL("Received message contained incorrect data.");
            ++client_received;

            client->get_session()->write(message);
        });
    }


    for (auto& [id, session] : server->get_sessions()) {
        session->write(message);
    }

    ve::steady_clock::time_point start = ve::steady_clock::now();
    while (ve::time_since(start) < ve::seconds(10)) {
        server->update();
        for (auto& client : clients) client->update();

        if (client_received == num_clients && server_received == num_clients) break;
    }


    result |= (client_received == num_clients && server_received == num_clients)
        ? VE_TEST_SUCCESS
        : VE_TEST_FAIL(
            "Not all expected messages were received: \n",
            "Client: ", client_received, " / ", num_clients, "\n",
            "Server: ", server_received, " / ", num_clients
        );

    return result;
}