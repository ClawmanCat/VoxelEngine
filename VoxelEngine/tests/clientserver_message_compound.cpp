#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/connect.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_compound.hpp>


using namespace ve::defs;


struct test_message {
    std::string text;
};


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    ve::client client;
    ve::server server;
    ve::connect_local(client, server);


    std::size_t callback_invoke_count = 0;

    client.get_server_connection()->add_raw_handler(
        "ve.test.test_message",
        [&] (const test_message& msg) {
            if (msg.text != "This is a test message.") result |= VE_TEST_FAIL("Received message had incorrect contents.");
            ++callback_invoke_count;
        }
    );



    auto connection = server.get_connections().front();

    ve::compound_message msg;
    msg.push_message("ve.test.test_message", test_message { "This is a test message." }, connection.get());
    msg.push_message("ve.test.test_message", test_message { "This is a test message." }, connection.get());

    connection->send_message(ve::core_message_types::MSG_COMPOUND, msg);


    if (callback_invoke_count != 2) {
        result |= VE_TEST_FAIL("Callback was not invoked for all messages in compound message (", callback_invoke_count, " / 2).");
    }


    return result;
}