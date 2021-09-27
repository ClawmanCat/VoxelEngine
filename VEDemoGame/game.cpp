#include <VEDemoGame/game.hpp>
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


    void game::pre_init(void) {
        game::window = ve::gfx::window::create(ve::gfx::window::arguments {
            .title = game::get_info()->display_name
        });

        auto shader   = ve::gfx::shader_cache::instance().template get_or_load_shader<vertex>("test_shader");
        auto pipeline = ve::make_shared<ve::gfxapi::rasterization_pipeline>(shader, game::window->get_canvas());

        auto vbo = ve::make_shared<ve::gfxapi::unindexed_vertex_buffer<vertex>>();
        vbo->update(std::array {
            vertex { ve::vec3f { -0.5f, -0.5f, 0.0f } },
            vertex { ve::vec3f { +0.5f, -0.5f, 0.0f } },
            vertex { ve::vec3f { +0.5f, +0.5f, 0.0f } }
        });

        game::client.add_system(ve::system_renderer<>{ std::move(pipeline) });
        game::client.create_entity(
            ve::transform_component { },
            ve::mesh_component { .mesh = std::move(vbo) }
        );
    }

    
    void game::post_init(void) {
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