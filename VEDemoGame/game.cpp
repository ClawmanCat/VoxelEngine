#include <VEDemoGame/game.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/entity/world.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/input/controls.hpp>

#include <VoxelEngine/clientserver/connect.hpp>
#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/utility/color.hpp>


namespace demo_game {
    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}
    void game::pre_init(void)  {}

    
    void game::post_init(void) {
        using simple_vertex_t = ve::gfx::vertex_types::texture_vertex_3d;
        using pbr_vertex_t    = ve::gfx::vertex_types::material_vertex_3d;


        // Set up render pipelines.
        game::window = ve::gfx::window::create(ve::gfx::window::arguments {
            .title = game::get_info()->display_name
        });

        game::texture_manager = ve::make_shared<ve::gfx::texture_manager<>>();


        // Simple pipeline for rendering objects without lighting.
        auto simple_pipeline = make_shared<ve::gfxapi::single_pass_pipeline>(
            game::window->get_canvas(),
            ve::gfx::shader_cache::instance().get_or_load_shader<simple_vertex_t>("simple")
        );

        simple_pipeline->set_uniform_producer(&game::camera);
        simple_pipeline->set_uniform_producer(game::texture_manager->get_atlas());


        // Pipeline for rendering objects with PBR materials.
        auto pbr_pipeline = make_shared<ve::gfxapi::single_pass_pipeline>(
            game::window->get_canvas(),
            ve::gfx::shader_cache::instance().get_or_load_shader<pbr_vertex_t>("pbr_single_pass")
        );

        pbr_pipeline->set_uniform_producer(&game::camera);
        pbr_pipeline->set_uniform_producer(game::texture_manager->get_atlas());


        // Set up ECS.
        using component_uniforms = ve::meta::pack<ve::transform_component>;
        using simple_renderer    = ve::meta::pack<render_tag_simple>;
        using pbr_renderer       = ve::meta::pack<render_tag_pbr>;

        game::client.add_system(ve::system_renderer<component_uniforms, simple_renderer> { simple_pipeline });
        game::client.add_system(ve::system_renderer<component_uniforms, pbr_renderer> { pbr_pipeline });
        game::client.add_system(ve::system_updater<> { });
        game::client.add_system(ve::system_physics<> { });
        game::client.add_system(ve::system_bind_camera<decltype(game::camera)> { });


        // Create entities.
        auto& entity_player = game::client.store_static_entity(player { game::client, &game::camera });
        auto& entity_world  = game::client.store_static_entity(world { game::client });

        for (ve::i32 x = -32; x < 32; ++x) {
            for (ve::i32 z = -32; z < 32; ++z) {
                if (x != 3 || z != 3) continue;

                howlee& h = game::client.store_static_entity(howlee { game::client });
                h.transform.position = ve::vec3f { x, 0.2f, z };
            }
        }


        // Add a light at the player's position.
        // TODO: Replace this with multipass PBR pipeline. Make lights into components.
        pbr_pipeline->set_uniform_producer<ve::gfx::lighting_data_uniform<>>("U_Lighting", [&] {
            ve::gfx::lighting_data_uniform result;

            result.lights[0] = ve::gfx::light_uniform {
                .position    = entity_player.transform.position,
                .radiance    = ve::normalize_color(ve::colors::GOLDENROD) * 50.0f,
                .attenuation = 1.25f
            };

            result.num_populated_lights = 1;
            result.ambient_light = ve::vec3f { 0.03f };

            return result;
        });


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