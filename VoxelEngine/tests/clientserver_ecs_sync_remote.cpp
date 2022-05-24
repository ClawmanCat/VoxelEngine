#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/connect.hpp>
#include <VoxelEngine/ecs/system/system_entity_visibility.hpp>
#include <VoxelEngine/ecs/system/system_synchronizer.hpp>

using namespace ve::defs;


struct test_component_1 {
    i32 x, y;
};

struct test_component_2 {
    std::string text;
};


test_result test_main(void) {
    ve::server server;
    ve::host_server(server, 6969);

    ve::client client;
    ve::connect_remote(client, "127.0.0.1", 6969);


    std::mutex mtx;
    test_result connection_errors = VE_TEST_SUCCESS;

    auto handler = [&] (const auto& e) {
        std::lock_guard lock { mtx };
        connection_errors |= VE_TEST_FAIL(e.error.message());
    };

    auto socket_server = server.template get_object<ve::shared<ve::connection::socket_server>>("ve.connection");
    socket_server->add_raw_handler<ve::connection::instance_error_event>(handler);
    socket_server->add_raw_handler<ve::connection::session_error_event>(handler);

    auto socket_client = client.template get_object<ve::shared<ve::connection::socket_client>>("ve.connection");
    socket_client->add_raw_handler<ve::connection::instance_error_event>(handler);
    socket_client->add_raw_handler<ve::connection::session_error_event>(handler);


    using SyncedComponents = ve::meta::pack<test_component_1, test_component_2>;

    auto [vis_id, visibility_system] = server.add_system(ve::system_entity_visibility { });
    auto [sync_id, sync_system] = server.add_system(ve::system_synchronizer<SyncedComponents> { visibility_system });


    std::vector<entt::entity> entities;
    entities.reserve(10);

    for (i32 i = 0; i < 10; ++i) {
        entt::entity e = server.create_entity();
        if (i % 2 == 0) server.set_component(e, test_component_1 { i, 2 * i });
        if (i % 4 == 0) server.set_component(e, test_component_2 { ve::to_string(i) });

        entities.push_back(e);
    }


    ve::steady_clock::time_point start = ve::steady_clock::now();

    while (ve::time_since(start) < ve::seconds(1)) {
        static ve::nanoseconds last = ve::seconds(1) / 60;
        ve::steady_clock::time_point tick_begin = ve::steady_clock::now();

        server.update(last);
        client.update(last);

        {
            std::lock_guard lock { mtx };
            if (connection_errors.error) return connection_errors;
        }

        last = time_since(tick_begin);
    }


    for (i32 i = 0; i < 10; ++i) {
        entt::entity e = entities[i];


        if (!client.get_storage().valid(e)) {
            return VE_TEST_FAIL("Entity ", e, " was not synchronized with client.");
        }


        if (i % 2 == 0) {
            const auto* component = client.try_get_component<test_component_1>(e);

            if (!component) {
                return VE_TEST_FAIL("Entity ", e, " is missing test_component_1.");
            }

            if (component->x != i) {
                return VE_TEST_FAIL("Entity ", e, " has incorrect value for test_component_1::x (Expected ", i, ", got ", component->x, ").");
            }

            if (component->y != 2 * i) {
                return VE_TEST_FAIL("Entity ", e, " has incorrect value for test_component_1::y (Expected ", 2 * i, ", got ", component->y, ").");
            }
        }


        if (i % 4 == 0) {
            const auto* component = client.try_get_component<test_component_2>(e);

            if (!component) {
                return VE_TEST_FAIL("Entity ", e, " is missing test_component_2.");
            }

            std::string expected_text = ve::to_string(i);
            if (component->text != expected_text) {
                return VE_TEST_FAIL("Entity ", e, " has incorrect value for test_component_2::text (Expected ", expected_text, ", got ", component->text, ").");
            }
        }
    }


    return VE_TEST_SUCCESS;
}