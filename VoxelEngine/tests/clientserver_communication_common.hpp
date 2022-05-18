#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>

#include <atomic>


using namespace ve::defs;


struct test_message {
    std::string msg;
};


test_result test_with_connection_method(
    const std::function<void(ve::client&, ve::server&)>& connect,
    const std::function<void(ve::client&, ve::server&)>& disconnect,
    std::size_t num_clients
) {
    std::vector<unique<ve::client>> clients;

    for (std::size_t i = 0; i < num_clients; ++i) {
        auto& client = clients.emplace_back(make_unique<ve::client>());
    }

    unique<ve::server> server = make_unique<ve::server>();


    const std::string msg_string = "This is a test message!";
    std::atomic_uint64_t received_on_server = 0, received_on_client = 0;


    auto server_handler = server->add_raw_handler([&](const ve::instance_connected_event& event) {
        server->get_client_connection(event.remote)->add_raw_handler(
            "ve.test.test_message",
            [&](const test_message& msg) {
                if (msg.msg == msg_string) ++received_on_server;
            }
        );

        server->get_client_connection(event.remote)->send_message("ve.test.test_message", test_message { msg_string });
    });


    std::vector<ve::client::handler_id> client_handlers;
    client_handlers.reserve(clients.size());

    for (auto& client : clients) {
        client_handlers.push_back(client->add_raw_handler([&](const ve::instance_connected_event& event) {
            client->get_server_connection()->add_raw_handler(
                "ve.test.test_message",
                [&](const test_message& msg) {
                    if (msg.msg == msg_string) ++received_on_client;
                }
            );

            client->get_server_connection()->send_message("ve.test.test_message", test_message { msg_string });
        }));
    }


    for (auto& client : clients) connect(*client, *server);


    // If the instances are communicating through a socket connection,
    // there might be some delay before the message is received.
    steady_clock::time_point start = steady_clock::now();
    steady_clock::time_point last  = start;

    while (time_since(start) < 10s && (received_on_server < clients.size() || received_on_client < clients.size())) {
        server->update(time_since(last));
        for (auto& client : clients) client->update(time_since(last));

        last = steady_clock::now();
    }


    for (auto& client : clients) disconnect(*client, *server);

    // Omission of '&' is intentional, zip returns pairs of references.
    for (auto [client, handler] : views::zip(clients, client_handlers)) {
        client->remove_handler<ve::instance_connected_event>(handler);
    }

    server->remove_handler<ve::instance_connected_event>(server_handler);


    if (received_on_server < clients.size() || received_on_client < clients.size()) {
        return VE_TEST_FAIL(
            "Not all expected messages were received: \n",
            "Client: ", received_on_client, " / ", clients.size(), "\n",
            "Server: ", received_on_server, " / ", clients.size()
        );
    }

    return VE_TEST_SUCCESS;
}