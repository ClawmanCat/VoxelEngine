#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    using namespace ve::namespaces;
    namespace gfx   = ve::graphics;
    namespace paths = ve::io::paths;
    
    
    class game {
    public:
        static void on_pre_init(void)  {}
        static void on_post_loop(void) {}
        static void on_pre_exit(void)  {}
        static void on_post_exit(void) {}
        
        
        static void on_post_init(void) {
            game::window = std::make_shared<gfx::window>(
                gfx::window::arguments { .title = get_info()->name.c_str() }
            );
            
            game::renderer = std::make_shared<gfx::pipeline>();
            
            
            // Exit if all windows are closed.
            ve_register_input_event(ve::last_window_closed_event, _, ve::engine::request_exit());
            
            
            // Add camera to renderer.
            game::camera.set_position(ve::vec3f { 0, 0, -3 });
            
            game::renderer->set_uniform_producer<ve::mat4f>(
                "camera"s,
                ve_get(game::camera.get_matrix())
            );
            
            
            // Update camera if window is resized.
            ve_register_input_event(
                ve::window_resize_event,
                evnt,
                if (evnt.window == game::window.get()) game::camera.set_aspect_ratio_for_size(evnt.new_size)
            );
            
            
            // Add cube to renderer.
            auto buffer = gfx::textured_cube(gfx::cube_textures {
                .bottom = *texture_manager.get_texture(ve::io::tile_resource("test_texture_A.png")),
                .top    = *texture_manager.get_texture(ve::io::tile_resource("test_texture_A.png")),
                .back   = *texture_manager.get_texture(ve::io::tile_resource("test_texture_B.png")),
                .front  = *texture_manager.get_texture(ve::io::tile_resource("test_texture_B.png")),
                .left   = *texture_manager.get_texture(ve::io::tile_resource("test_texture_C.png")),
                .right  = *texture_manager.get_texture(ve::io::tile_resource("test_texture_C.png"))
            });
            
            game::renderer->add_buffer(
                gfx::shader_library::get_shader("atlas_texture_3d"s),
                std::move(buffer)
            );
            
            
            // Set renderer to draw to the window at z-index 0.
            game::window->add_layer(
                gfx::layerstack_target { game::renderer },
                0.0f
            );
        }
    
        
        static void on_pre_loop(void)  {
            // Rotate camera around origin.
            const float speed = 0.0005f;
            const ve::vec3f target = { 0, 0, 0 };
            
            game::camera.move(game::camera.get_right() * speed);
            game::camera.set_y(sin(ve::engine::get_tick_count() * speed));
            
            game::camera.look_at(target);
        }
    
        
        [[nodiscard]] static const ve::game_info* get_info(void) {
            static const ve::game_info info {
                .name         = "VoxelEngine Demo Game",
                .description  = { "Example game to demonstrate basic functionality." },
                .authors      = { "ClawmanCat" },
                .game_version = {
                    "PreAlpha",
                    VEDEMOGAME_VERSION_MAJOR,
                    VEDEMOGAME_VERSION_MINOR,
                    VEDEMOGAME_VERSION_PATCH
                }
            };
            
            return &info;
        }
    
        
        [[nodiscard]] static gfx::window& get_window(void) { return *window; }
        [[nodiscard]] static gfx::pipeline& get_renderer(void) { return *renderer; }
    private:
        static inline ve::shared<gfx::window> window = nullptr;
        static inline ve::shared<gfx::pipeline> renderer = nullptr;
        static inline gfx::perspective_camera camera;
        static inline gfx::texture_manager<> texture_manager;
    };
}


// Callbacks for the game engine.
namespace ve::game_callbacks {
    void on_game_pre_init(void)  { demo_game::game::on_pre_init();  }
    void on_game_post_init(void) { demo_game::game::on_post_init(); }
    void on_game_pre_loop(void)  { demo_game::game::on_pre_loop();  }
    void on_game_post_loop(void) { demo_game::game::on_post_loop(); }
    void on_game_pre_exit(void)  { demo_game::game::on_pre_exit();  }
    void on_game_post_exit(void) { demo_game::game::on_post_exit(); }
    
    const ve::game_info* get_game_info(void) {
        return demo_game::game::get_info();
    }
}