#include <VEDemoGame/game.hpp>
#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/component/render_tags.hpp>

#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/input/input.hpp>
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


        // Set up graphics pipelines.
        game::window          = window::create(window::arguments { .title = game::get_info()->display_name });
        game::texture_manager = make_shared<ve::gfx::texture_manager<>>();


        auto simple_pipeline = make_shared<single_pass_pipeline>(
            game::window->get_canvas(),
            shader_cache::instance().template get_or_load_shader<simple_vertex>("simple")
        );

        auto pbr_pipeline = multipass_pbr_pipeline::create(game::window->get_canvas());

        std::tuple pipelines {
            std::tuple { simple_pipeline, simple_render_tag { } },
            std::tuple { pbr_pipeline,    pbr_render_tag    { } }
        };

        tuple_foreach(pipelines, [&] <typename P> (P& pair) {
            auto& pipeline     = std::get<0>(pair);
            using pipeline_tag = std::tuple_element_t<1, P>;

            pipeline->set_uniform_producer(&game::camera);
            pipeline->set_uniform_producer(game::texture_manager->get_atlas());

            auto [id, system] = game::client->add_system(system_renderer<
                ve::meta::pack<transform_component>, // Uniforms
                ve::meta::pack<pipeline_tag>         // Required tags
            > { pipeline });

            system.get_lighting().ambient_light = 10.0f * normalize_color(colors::LIGHT_GOLDENROD_YELLOW);
        });


        // Set up user input handling.
        input_manager::instance().add_handler([] (const window_resized_event& e) {
            if (e.window == game::window.get()) {
                auto size = vec2f { e.new_size };
                game::camera.set_aspect_ratio(size.x / size.y);
            }
        });

        controls.bind("capture_mouse", [](const binary_input::handler_args& args) { input_manager::instance().set_mouse_capture(true);  });
        controls.bind("release_mouse", [](const binary_input::handler_args& args) { input_manager::instance().set_mouse_capture(false); });
        input_manager::instance().set_mouse_capture(true);

        game::client->add_system(system_bind_camera<decltype(camera)> { });
        game::client->add_system(system_updater<> { });
        game::client->add_system(system_physics<> { });


        // Set up synchronization.
        auto [visibility_id,  visibility_system ] = game::client->add_system(system_set_visibility { });
        auto [remote_init_id, remote_init_system] = game::client->add_system(system_remote_init { });

        remote_init_system.add_initializer(&player::remote_initializer);
        remote_init_system.add_initializer(&howlee::remote_initializer);


        // Client only needs to send the transform of player-controlled entity.
        game::client->add_system(system_synchronizer<
            ve::meta::pack<transform_component, motion_component>,
            ve::meta::pack<player::local_player_tag>
        > { &visibility_system });


        // Ignore changes from the remote to the local player's position.
        // TODO: Implement a better way to manage cases like this.
        game::client->get_validator().add_rule([] (instance_id remote, registry& r, entt::entity e, const transform_component* o, const transform_component* n) {
            return !(r.has_component<player::local_player_tag>(e));
        });


        // Player uses motion system to modify its own transform.
        game::client->add_system(system_physics { });


        // Connect to server.
        auto address = engine::get_arguments().template get<std::string>("remote_address");
        VE_ASSERT(address, "When running the client, please specify a remote address to connect to with --remote_address=<address>");

        connect_remote(*game::client, *address, 6969);
    }


    void game::setup_server(void) {
        game::server = make_unique<class server>();


        // Set up server-side systems.
        game::server->add_system(ve::system_updater<> { });
        game::server->add_system(ve::system_physics<> { });


        // Set up synchronization.
        auto [visibility_id, visibility_system] = game::server->add_system(system_set_visibility { });


        using synchronized_components = ve::meta::pack<
            transform_component,
            motion_component,
            light_component,
            remote_init_component
        >;


        game::server->add_system(system_synchronizer<synchronized_components> { &visibility_system });


        // Add player entity when a player connects.
        game::server->add_handler([] (const instance_connected_event& e) {
            game::server->store_static_entity(player { *game::server, e.remote });
        });


        // Allow players to update their own position and velocity.
        auto rule = [] (instance_id remote, registry& r, entt::entity e, const auto* prev, const auto* current) {
            const auto* player_id = r.template try_get_component<typename player::player_id>(e);

            if (player_id && player_id->player == remote) return true;
            else return false;
        };

        game::server->get_validator().add_rule<decltype(rule), transform_component>(rule);
        game::server->get_validator().add_rule<decltype(rule), motion_component>(rule);


        // For now, allow the client to observe all entities.
        game::server->set_default_visibility(true);


        // Create entities.
        for (i32 x = -5; x <= +5; ++x) {
            for (i32 y = -5; y <= +5; ++y) {
                if (x == 0 && y == 0) continue; // Skip the howlee on top of the player.

                auto& h = game::server->store_static_entity(howlee { *game::server });
                h.transform.position = vec3f { x, 0, y };
            }
        }


        // Host server.
        host_server(*game::server, 6969);
    }
}