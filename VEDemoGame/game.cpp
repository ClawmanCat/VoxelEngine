#include <VEDemoGame/game.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/input/controls.hpp>

#include <VoxelEngine/clientserver/connect.hpp>
#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}
    void game::pre_init(void)  {}

    
    void game::post_init(void) {
        using vertex_t = ve::gfx::vertex_types::texture_vertex_3d;


        // Set up render pipeline.
        game::window = ve::gfx::window::create(ve::gfx::window::arguments {
            .title = game::get_info()->display_name
        });

        game::texture_manager = ve::make_shared<ve::gfx::texture_manager<>>();

        auto pipeline = make_shared<ve::gfxapi::single_pass_pipeline>(
            game::window->get_canvas(),
            ve::gfx::shader_cache::instance().get_or_load_shader<vertex_t>("simple")
        );

        pipeline->set_uniform_producer(&game::camera);
        pipeline->set_uniform_producer(game::texture_manager->get_atlas());


        // Set up ECS.
        game::client.add_system(ve::system_renderer<> { std::move(pipeline) });
        game::client.add_system(ve::system_updater<> { });
        game::client.add_system(ve::system_physics<> { });
        game::client.add_system(ve::system_bind_camera<decltype(game::camera)> { });


        // Create entities.
        game::client.store_static_entity(player { game::client, &game::camera });

        for (ve::i32 x = -32; x < 32; ++x) {
            for (ve::i32 z = -32; z < 32; ++z) {
                if (x != 3 || z != 3) continue;

                howlee& h = game::client.store_static_entity(howlee { game::client });
                h.transform.position = ve::vec3f { x, 0.2f, z };
            }
        }


        auto floor_texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_TILE_TEXTURES / "hd_grass_color.png");
        auto floor_buffer  = ve::gfx::textured_quad(ve::vec2f { 100.0f }, floor_texture);

        game::client.create_entity(
            ve::mesh_component { std::move(floor_buffer) },
            ve::transform_component {
                .position = ve::vec3f { 0, -0.1f, 0 },
                .rotation = glm::angleAxis(glm::radians(90.0f), ve::direction::RIGHT) }
        );


        // Set up non-player-control hotkeys.
        controls.bind("capture_mouse", [](const ve::binary_input::handler_args& args) { ve::input_manager::instance().set_mouse_capture(true);  });
        controls.bind("release_mouse", [](const ve::binary_input::handler_args& args) { ve::input_manager::instance().set_mouse_capture(false); });
        ve::input_manager::instance().set_mouse_capture(true);


        // Connect client & server.
        ve::connect_local(game::client, game::server);
    }
    
    
    const ve::game_info* game::get_info(void) {
        const static ve::game_info info {
            .display_name = "Demo Game",
            .description  = { "Demonstrator to show engine functionality." },
            .authors      = { "ClawmanCat" },
            .version      = ve::version {
                VEDEMOGAME_VERSION_MAJOR,
                VEDEMOGAME_VERSION_MINOR,
                VEDEMOGAME_VERSION_PATCH,
                "PreAlpha"
            }
        };
    
        return &info;
    }
}