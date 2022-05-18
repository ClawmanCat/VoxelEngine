#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/clientserver/clientserver.hpp>
#include <VoxelEngine/ecs/ecs.hpp>


struct component { int value; };


// Assure the client cannot edit a component locally, then update the remote when this is not explicitly allowed.
test_result illegal_action_doesnt_modify_remote(void) {
    constexpr ve::nanoseconds dt = ve::seconds { 1 };

    ve::client client;
    ve::server server;
    ve::connect_local(client, server);

    for (auto& instance : std::array<ve::instance*, 2> { &client, &server }) {
        auto [vis_id, vis_system] = instance->add_system(ve::system_entity_visibility { });
        instance->add_system(ve::system_synchronizer<ve::meta::pack<component>> { vis_system });
    }

    auto entity = server.create_entity(component { 11 });

    server.update(dt); // Server synchronizes entity here.
    client.update(dt); // Client receives entity here.


    client.template get_component<component>(entity).value = 22;

    server.update(dt);
    client.update(dt); // Change is sent to server here.
    server.update(dt); // Server processes change and responds here.
    client.update(dt); // Client receives response here.


    if (server.template get_component<component>(entity).value != 11) {
        return VE_TEST_FAIL("Client was able to change server component without permission.");
    } else {
        return VE_TEST_SUCCESS;
    }
}


// Assure the server restores the state of the client after it attempts to make an illegal modification.
test_result illegal_action_is_reverted(void) {
    constexpr ve::nanoseconds dt = ve::seconds { 1 };

    ve::client client;
    ve::server server;
    ve::connect_local(client, server);


    for (auto& instance : std::array<ve::instance*, 2> { &client, &server }) {
        auto [vis_id,  vis_system ] = instance->add_system(ve::system_entity_visibility { });
        auto [sync_id, sync_system] = instance->add_system(ve::system_synchronizer<ve::meta::pack<component>> { vis_system });

        if (instance->get_type() == ve::instance::SERVER) {
            // Mark component as visible so the server will revert the change rather than ignoring it.
            instance->get_validator().set_default_for_synced_components(&sync_system, ve::change_result::FORBIDDEN);

            // Prevent false positives from the value being updated through the sync system rather than being reverted directly.
            sync_system.set_sync_rate<component>(ve::seconds { 1000 });
        } else {
            // Note: client still needs to sync so we can check the server undoes the change when it receives it.
            sync_system.set_sync_rate<component>(ve::seconds { 0 });
        }
    }


    auto entity = server.create_entity(component { 11 });

    server.update(dt); // Server synchronizes entity here.
    client.update(dt); // Client receives entity here.


    client.template get_component<component>(entity).value = 22;


    server.update(dt);
    client.update(dt); // Change is sent to server here.
    server.update(dt); // Server processes change and responds here.
    client.update(dt); // Client receives response here.


    if (client.template get_component<component>(entity).value != 11) {
        return VE_TEST_FAIL("Illegal action on client was not undone by server.");
    } else {
        return VE_TEST_SUCCESS;
    }
}


// Assure an attempt to modify a component not visible to the local instance doesn't leak remote data.
test_result illegal_action_doesnt_leak_data(void) {
    constexpr ve::nanoseconds dt = ve::seconds { 1 };

    ve::client client;
    ve::server server;
    ve::connect_local(client, server);

    auto entity = server.create_entity(component { 11 });

    server.update(dt);
    client.update(dt);


    // Send fake message to change component & await response.
    bool received_response = false;

    client.get_server_connection()->add_raw_handler(
        ve::core_message_types::MSG_UNDO_COMPONENT,
        [&] (const ve::undo_component_message& value) { received_response = true; }
    );

    client.get_server_connection()->send_message(
        ve::core_message_types::MSG_SET_COMPONENT,
        ve::set_component_message {
            .component_data = ve::serialize::to_bytes(component { 22 }),
            .component_type = ve::type_hash<component>(),
            .entity = entity
        }
    );


    server.update(dt);
    client.update(dt);


    if (received_response) {
        return VE_TEST_FAIL("Client was able to leak component data from server.");
    } else {
        return VE_TEST_SUCCESS;
    }
}


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    result |= illegal_action_doesnt_modify_remote();
    result |= illegal_action_is_reverted();
    result |= illegal_action_doesnt_leak_data();

    return result;
}

