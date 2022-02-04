#include <VEDemoGame/game.hpp>
#include <VEDemoGame/input/input_binder.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/entity/player.hpp>

#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/clientserver/connect.hpp>


namespace demo_game {
    using namespace ve;
    using namespace ve::gfx;
    using namespace ve::gfxapi;

    using simple_vertex = vertex_types::texture_vertex_3d;
    using pbr_vertex    = vertex_types::material_vertex_3d;


    void game::pre_init(void)  {}
    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}


    const game_info* game::get_info(void) {
        const static game_info info {
            .display_name = "Demo Game",
            .description  = { "Demonstrator to show engine functionality." },
            .authors      = { "ClawmanCat" },
            .version      = version {
                VEDEMOGAME_VERSION_MAJOR,
                VEDEMOGAME_VERSION_MINOR,
                VEDEMOGAME_VERSION_PATCH,
                "PreAlpha"
            }
        };

        return &info;
    }


    void game::post_init(void) {
        if (engine::get_arguments().has("server")) setup_server();
        if (engine::get_arguments().has("client")) setup_client();
    }


    void game::setup_client(void) {
        game::client = make_unique<class client>();

        game::setup_client_graphics();
        game::setup_client_input();
        game::setup_client_synchronization();
        game::setup_client_systems();


        // Connect to server.
        auto remote_address = engine::get_arguments().value_or<std::string>("remote_address", "127.0.0.1");
        connect_remote(*game::client, remote_address, 6969);

        auto socket_connection = game::client->get_object<shared<connection::socket_client>>("ve.connection");
        socket_connection->add_handler([] (const connection::session_error_event& e) {
            thread_pool::instance().invoke_on_thread([e] {
                auto msg = "An error occurred in the connection with the server. The connection was terminated.\n"s + e.error.message();

                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    "Connection Error",
                    msg.c_str(),
                    nullptr
                );
            });
        });
    }


    void game::setup_client_graphics(void) {
        game::window          = window::create(window::arguments { .title = game::get_info()->display_name });
        game::texture_manager = make_shared<ve::gfx::texture_manager<>>();


        auto simple_pipeline = make_shared<single_pass_pipeline>(game::window->get_canvas(), shader_cache::instance().get_or_load_shader<simple_vertex>("simple"));
        auto pbr_pipeline    = multipass_pbr_pipeline::create(game::window->get_canvas());

        std::tuple pipelines {
            std::tuple { simple_pipeline, simple_render_tag {} },
            std::tuple { pbr_pipeline,    pbr_render_tag    {} }
        };


        tuple_foreach(pipelines, [&] <typename KV> (KV& kv) {
            auto& pipeline   = std::get<0>(kv);
            using render_tag = std::tuple_element_t<1, KV>;

            pipeline->set_uniform_producer(&game::camera);
            pipeline->set_uniform_producer(game::texture_manager->get_atlas());

            auto& system = game::client->add_system(system_renderer<
                ve::meta::pack<transform_component>,
                ve::meta::pack<render_tag>
            > { pipeline }).second;

            system.get_lighting().ambient_light = 25.0f * normalize_color(colors::LIGHT_GOLDENROD_YELLOW);
        });
    }


    void game::setup_client_input(void) {
        // Handle mouse capture and release.
        controls.bind("capture_mouse", [](const binary_input::handler_args& args) { input_manager::instance().set_mouse_capture(true);  });
        controls.bind("release_mouse", [](const binary_input::handler_args& args) { input_manager::instance().set_mouse_capture(false); });
        input_manager::instance().set_mouse_capture(true);


        // Update the camera when the window is resized.
        input_manager::instance().add_handler([] (const window_resized_event& e) {
            if (e.window == game::window.get()) {
                auto size = vec2f { e.new_size };
                game::camera.set_aspect_ratio(size.x / size.y);
            }
        });
    }


    void game::setup_client_synchronization(void) {
        // TODO: Ignore server changes to player position unless it becomes out of sync.

        // Run initializers for newly created entities.
        auto [init_id, init_system] = game::client->add_system(system_remote_initializer {});
        init_system.add_initializer(&player::remote_initializer);
        init_system.add_initializer(&howlee::remote_initializer);

        // Synchronize player motion with server.
        auto [vis_id, vis_system] = game::client->add_system(system_entity_visibility { produce(true) });

        game::client->add_system(system_synchronizer<
            ve::meta::pack<transform_component, motion_component>,  // Sync transform & motion,
            std::remove_cvref_t<decltype(vis_system)>,              // using the vis system,
            ve::meta::pack<typename player::local_player_tag>       // but only for the player controlled by this client.
        > { vis_system });
    }


    void game::setup_client_systems(void) {
        // Client gets its own physics system to continue simulating movement when no message from the server is received,
        // and to move the player according to the provided user input.
        game::client->add_system(system_physics<> {});

        // Allow client-side entities to define update script components (e.g. to update player movements).
        game::client->add_system(system_updater<> {});
    }


    void game::setup_server(void) {
        game::server = make_unique<class server>();

        game::setup_server_synchronization();
        game::setup_server_systems();
        game::setup_server_entities();

        // Begin accepting connections from clients.
        host_server(*game::server, 6969);
    }


    void game::setup_server_synchronization(void) {
        using synced_components = ve::meta::pack<
            transform_component,
            motion_component,
            typename player::player_controller,
            typename howlee::entity_howlee_tag
        >;


        // Allow clients to see all positionless entities and entities within 25 meters of them.
        auto visibility_rule = [] (registry& owner, entt::entity entity, message_handler* connection) {
            const auto* entity_transform = owner.try_get_component<transform_component>(entity);
            const auto* player_transform = owner.try_get_component<transform_component>(player::server_players[connection->get_remote_id()]);

            if (entity_transform && player_transform) {
                return glm::distance(entity_transform->position, player_transform->position) < 25.0f;
            } else return true;
        };


        auto [vis_id,  vis_system ] = game::server->add_system(system_entity_visibility { visibility_rule });
        auto [sync_id, sync_system] = game::server->add_system(system_synchronizer<
            synced_components,
            std::remove_cvref_t<decltype(vis_system)>
        > { vis_system });


        // Clients will determine position and velocity of their associated player, so don't sync these values.
        // TODO: Instead of this, server should sync the values, but client should ignore it, unless the two significantly disagree.
        auto rule = [] (instance_id remote, registry& owner, entt::entity entity, const auto& cmp) {
            static hash_set<instance_id> initial_sync_completed;

            const auto& player = owner.template get_component<typename player::player_controller>(entity);

            if (player.controlled_by == remote) {
                // If this is the remote controlling the player, send the value exactly once to initialize it.
                auto [it, success] = initial_sync_completed.emplace(remote);
                return success;
            } else return true;
        };

        using rule_requires = ve::meta::pack<typename player::player_controller>;
        sync_system.add_per_entity_rule<decltype(rule), transform_component, rule_requires>(rule);
        sync_system.add_per_entity_rule<decltype(rule), motion_component, rule_requires>(rule);
    }


    void game::setup_server_systems(void) {
        game::server->add_system(system_physics<> {});
        game::server->add_system(system_updater<> {});
    }


    void game::setup_server_entities(void) {
        // Create Howlees.
        for (i32 x = -5; x <= +5; ++x) {
            for (i32 z = -5; z <= +5; ++z) {
                auto& h = game::server->store_static_entity(howlee { *game::server });
                h.transform.position = vec3f { x, 0, z };
            }
        }


        // Create player entity when a client connects to the server.
        game::server->add_handler([] (const instance_connected_event& e) {
            game::server->store_static_entity(player { *game::server, e.remote });
            game::server_logger.info(cat("Player ", e.remote, " connected to the server."));
        });

        // Remove player entity when a client disconnects from the server.
        game::server->add_handler([] (const instance_disconnected_event& e) {
            game::server->destroy_entity(player::server_players[e.remote]);
            game::server_logger.info(cat("Player ", e.remote, " disconnected from the server."));
        });
    }
}