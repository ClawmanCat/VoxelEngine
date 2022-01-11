#include <VEDemoGame/game.hpp>
#include <VEDemoGame/input/input_binder.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/howlee.hpp>

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


        // Set up remote initializers for entity classes.
        // TODO: Refactor this to remote init system.
        game::client->add_handler([] (const entity_created_event& e) {
            game::client->set_component(e.entity, howlee::make_mesh());
            game::client->set_component(e.entity, simple_render_tag { });
        });


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


        // TODO: Move this to player entity.
        constexpr float player_speed            = 10.0f;
        constexpr float player_look_sensitivity = 1.0f;
        constexpr float epsilon                 = 1e-6f;

        constexpr std::array player_movements {
            std::pair { "move_forwards"sv,  direction::FORWARD  },
            std::pair { "move_backwards"sv, direction::BACKWARD },
            std::pair { "move_left"sv,      direction::LEFT     },
            std::pair { "move_right"sv,     direction::RIGHT    },
            std::pair { "move_up"sv,        direction::UP       },
            std::pair { "move_down"sv,      direction::DOWN     }
        };

        for (const auto& [input, direction] : player_movements) {
            controls.bind(input, [direction = vec3f { direction }](const binary_input::handler_args& args) {
                float dt = float(game::client->get_last_dt().count()) / 1e9f;
                game::camera.move((direction * player_speed * dt) * game::camera.get_rotation());
            });
        }

        controls.bind("look", [](const motion_input::handler_args& args) {
            static vec2f mouse_orientation = vec2f { 0 };

            mouse_orientation +=
                vec2f { args.current.position - args.prev.position } /
                vec2f { args.window->get_canvas_size() } *
                constants::f32_pi *
                player_look_sensitivity;

            mouse_orientation.y = std::clamp(
                mouse_orientation.y,
                -constants::f32_half_pi + epsilon,
                +constants::f32_half_pi - epsilon
            );

            quatf pitch = glm::angleAxis(mouse_orientation.y, (vec3f) direction::RIGHT);
            quatf yaw   = glm::angleAxis(mouse_orientation.x, (vec3f) direction::UP);

            game::camera.set_rotation(glm::normalize(pitch * yaw));
        });
    }


    void game::setup_client_synchronization(void) {
        // TODO: Send player position and velocity to server.
        // TODO: Ignore server changes to player position unless it becomes out of sync.
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
            motion_component
        >;


        auto [vis_id, vis_system] = game::server->add_system(system_entity_visibility { produce(true) });
        game::server->add_system(system_synchronizer<synced_components, std::remove_cvref_t<decltype(vis_system)>>{ vis_system });
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
    }
}