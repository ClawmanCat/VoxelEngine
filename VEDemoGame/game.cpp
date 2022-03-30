#include <VEDemoGame/game.hpp>
#include <VEDemoGame/input/key_bindings.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/entity/world.hpp>

#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/utility/random.hpp>
#include <VoxelEngine/clientserver/connect.hpp>


namespace demo_game {
    using namespace ve;
    using namespace ve::gfx;
    using namespace ve::gfxapi;

    using simple_vertex = vertex_types::texture_vertex_3d;
    using pbr_vertex    = vertex_types::material_vertex_3d;


    void game::pre_init(void)  {}
    void game::pre_loop(void)  {}
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


    void game::post_loop(void) {
        // Allow running for a certain number of ticks for CI purposes.
        if (auto run_for = engine::get_arguments().get<i64>("run_for"); run_for) {
            if (engine::get_tick_count() >= u64(*run_for)) {
                VE_LOG_INFO("Test run completed. Exiting...");
                engine::exit();
            }
        }
    }


    void game::setup_client(void) {
        game::client = make_unique<class client>();

        game::setup_client_graphics();
        game::setup_client_input();
        game::setup_client_synchronization();
        game::setup_client_systems();
        game::setup_client_connection();
    }


    void game::setup_client_graphics(void) {
        game::window          = window::create(window::arguments { .title = cat(game::get_info()->display_name, " - ", game::get_info()->version) });
        game::texture_manager = make_shared<ve::gfx::texture_manager<>>();


        auto simple_pipeline = make_shared<single_pass_pipeline>(game::window->get_canvas(), shader_cache::instance().get_or_load<simple_vertex>("simple"));
        auto pbr_pipeline    = pbr_pipeline::create(game::window->get_canvas());

        std::tuple pipelines {
            std::tuple { simple_pipeline, simple_render_tag {} },
            std::tuple { pbr_pipeline,    pbr_render_tag    {} }
        };


        tuple_foreach(pipelines, [&] <typename KV> (KV& kv) {
            auto& pipeline   = std::get<0>(kv);
            using render_tag = std::tuple_element_t<1, KV>;

            pipeline->set_uniform_producer(&game::camera);
            pipeline->set_uniform_producer(game::texture_manager->get_atlas());


            // Add lighting support for PBR renderer.
            if constexpr (std::is_same_v<render_tag, pbr_render_tag>) {
                auto& system = game::client->add_system(system_renderer<
                    ve::meta::pack<transform_component>,
                    ve::meta::pack<render_tag>,
                    ve::meta::pack<>,
                    renderer_mixins::lighting_mixin_for<>::template type, // Use all entities with light components as lights.
                    renderer_mixins::bloom_mixin
                > { pipeline }).second;

                system.set_ambient_light(0.25f * normalize_color(colors::LIGHT_GOLDENROD_YELLOW));
            } else {
                game::client->add_system(system_renderer<
                    ve::meta::pack<transform_component>,
                    ve::meta::pack<render_tag>
                > { pipeline });
            }
        });
    }


    void game::setup_client_input(void) {
        // Handle mouse capture and release.
        controls.add_binding("capture_mouse", [] { input_manager::instance().set_mouse_capture(true);  });
        controls.add_binding("release_mouse", [] { input_manager::instance().set_mouse_capture(false); });
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
        // Run initializers for newly created entities.
        auto [init_id, init_system] = game::client->add_system(system_remote_initializer {});
        init_system.add_initializer(&player::remote_initializer);
        init_system.add_initializer(&howlee::remote_initializer);
        init_system.add_initializer(&world::remote_initializer);


        // Synchronize player motion with server.
        auto [vis_id, vis_system] = game::client->add_system(system_entity_visibility { });

        game::client->add_system(system_synchronizer<
            ve::meta::pack<transform_component, motion_component>,  // Sync transform & motion,
            ve::meta::pack<typename player::local_player_tag>       // but only for the player controlled by this client.
        > { vis_system });


        // Discard changes to player position and velocity received from the server.
        auto rule = [] (instance_id remote, const registry& r, entt::entity e, const auto* old_value, const auto* new_value) {
            // Note: we have to allow the server to add the component when the player is created (!old_value).
            return !old_value || !r.template has_component<player::local_player_tag>(e);
        };

        game::client->get_validator().template add_rule_for<transform_component>(rule);
        game::client->get_validator().template add_rule_for<motion_component>(rule);


        // Automatically deny any changes to invisibly entities.
        // This is optional, since the server can already see the entire state of the client.
        game::client->get_validator().set_visibility_system(&vis_system);
    }


    void game::setup_client_systems(void) {
        // Client gets its own physics system to continue simulating movement between server messages,
        // and to move the player according to the provided user input.
        game::client->add_system(system_physics<> {});

        // Allow client-side entities to define update script components (e.g. to update player movements).
        game::client->add_system(system_updater<> {});
    }


    void game::setup_client_connection(void) {
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

                disconnect_remote(*game::client);
            });
        });
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
            voxel_component,
            light_component,
            typename player::player_controller,
            typename howlee::entity_howlee_tag
        >;


        // Allow clients to see all positionless entities, all voxel spaces, and entities within 200 meters of them.
        auto visibility_rule = [] (const registry& owner, entt::entity entity, const message_handler* connection) {
            if (owner.template has_component<voxel_component>(entity)) { return true; }

            const auto* entity_transform = owner.try_get_component<transform_component>(entity);
            const auto* player_transform = owner.try_get_component<transform_component>(player::server_players[connection->get_remote_id()]);

            if (entity_transform && player_transform) {
                return glm::distance(entity_transform->position, player_transform->position) < 200.0f;
            } else return true;
        };

        auto [vis_id,  vis_system ] = game::server->add_system(system_entity_visibility<> { visibility_rule });
        auto [sync_id, sync_system] = game::server->add_system(system_synchronizer<synced_components> { vis_system });


        // Allow clients to edit their position and velocity.
        auto rule = [] (instance_id remote, const registry& r, entt::entity e, const auto* old_value, const auto* new_value) {
            if (!old_value || !new_value) return false; // Adding or removing components is not allowed.

            if (const auto* player_tag = r.template try_get_component<typename player::player_controller>(e); player_tag) {
                return player_tag->controlled_by == remote;
            }

            return false;
        };

        game::server->get_validator().template add_rule_for<transform_component>(rule);
        game::server->get_validator().template add_rule_for<motion_component>(rule);


        // Automatically deny any changes to invisibly entities.
        // This is optional, since the default behaviour already disallows changes to anything other than the player itself.
        game::server->get_validator().set_visibility_system(&vis_system);

        // For synchronized components, inform the client if it tries to make an illegal change, rather than just ignoring it.
        game::server->get_validator().set_default_for_synced_components(&sync_system, change_result::FORBIDDEN);
    }


    void game::setup_server_systems(void) {
        game::server->add_system(system_physics<> {});
        game::server->add_system(system_updater<> {});
    }


    void game::setup_server_entities(void) {
        // Create world.
        auto& w = game::server->store_static_entity(world { *game::server });


        // Create Howlees.
        for (i32 x = -10; x <= +10; ++x) {
            for (i32 z = -10; z <= +10; ++z) {
                auto& h = game::server->store_static_entity(howlee { *game::server, &w });
                h.transform.position = vec3f { x, 0, z };
            }
        }


        // Create player entity when a client connects to the server.
        static hash_map<instance_id, shared<voxel::entity_loader<>>> entity_loaders;
        const static voxel::tilepos range { 3 };

        game::server->add_handler([&w] (const instance_connected_event& e) {
            auto& p = game::server->store_static_entity(player { *game::server, e.remote });
            game::server_logger.info(cat("Player ", e.remote, " connected to the server."));

            // Add chunk loader for player.
            auto [it, success] = entity_loaders.emplace(e.remote, make_shared<voxel::entity_loader<>>(p.get_id(), *game::server, range));
            w.voxel.get_space()->add_chunk_loader(it->second);
        });

        // Remove player entity when a client disconnects from the server.
        game::server->add_handler([&w] (const instance_disconnected_event& e) {
            auto it = entity_loaders.find(e.remote);
            w.voxel.get_space()->remove_chunk_loader(it->second);
            entity_loaders.erase(it);

            game::server->destroy_entity(player::server_players[e.remote]);
            game::server_logger.info(cat("Player ", e.remote, " disconnected from the server."));
        });
    }
}