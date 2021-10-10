#include <VEDemoGame/game.hpp>
#include <VEDemoGame/entity/howlee.hpp>

#include <VoxelEngine/clientserver/connect.hpp>


namespace demo_game {
    struct vertex {
        ve::vec3f position;
        ve_vertex_layout(vertex, position);
    };


    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}
    void game::pre_init(void)  {}

    
    void game::post_init(void) {
        using vertex_t = ve::gfx::vertex_types::texture_vertex_3d;


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

        game::client.add_system(ve::system_renderer<> { std::move(pipeline) });
        game::client.add_system(ve::system_updater<> { });

        for (ve::i32 x = -32; x < 32; ++x) {
            for (ve::i32 z = -32; z < 32; ++z) {
                howlee h { game::client };
                h.transform.set_position(ve::vec3f { x, 0, z });

                game::client.store_static_entity(std::move(h));
            }
        }


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