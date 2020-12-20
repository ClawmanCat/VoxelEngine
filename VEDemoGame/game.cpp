#include <VEDemoGame/game.hpp>

#include <VoxelEngine/graphics/render/buffer/vertex_array.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex.hpp>
#include <VoxelEngine/graphics/render/mesh/meshes.hpp>
#include <VoxelEngine/graphics/render/camera/perspective_camera.hpp>
#include <VoxelEngine/graphics/render/shader/shader_library.hpp>
#include <VoxelEngine/graphics/render/target/layerstack_target.hpp>
#include <VoxelEngine/graphics/render/graphics_pipeline.hpp>
#include <VoxelEngine/graphics/render/texture/aligned_texture_atlas.hpp>
#include <VoxelEngine/graphics/layerstack.hpp>
#include <VoxelEngine/graphics/window.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/utils/io/io.hpp>
#include <VoxelEngine/utils/io/paths.hpp>
#include <VoxelEngine/utils/color.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/engine_event.hpp>


namespace demo_game {
    void game::on_pre_init(void) noexcept {
        ve::engine::get_dispatcher().add_handler<ve::engine_post_sdl_init_event>(
            ve::events::event_handler { [](const ve::events::event& e) {
                VE_LOG_DEBUG("Creating window for "s + game::get_info()->name + ".");
                
                game::window_id = ve::window_manager::instance()
                    .create_window({ game::get_info()->name.c_str() })
                    .lock()
                    ->get_id();
                
                // Exit the engine if the window is closed.
                ve::input_manager::instance().get_dispatcher().add_handler<ve::window_closed_event>(
                    ve::events::event_handler {
                        [](const auto& e) {
                            const auto& evnt = static_cast<const ve::window_closed_event&>(e);
                            
                            if (evnt.window_id == game::window_id) {
                                ve::engine::exit();
                            }
                        }
                    }
                );
            }}
        );
    }
   
    
    void game::on_post_init(void) noexcept {
        // Below is example code to render a cube to the screen.
        using vertex = ve::vertex::flat::texture_vertex_3d;
        
        // Initialize texture atlas.
        // TODO: Store atlases as resource_owner.
        atlas = new ve::aligned_texture_atlas { };
        
        // Create a pipeline to render the cube with.
        pipeline = std::make_shared<ve::graphics_pipeline>();
        
        // Load textures.
        std::array<ve::subtexture, 3> textures;
        
        char ch = 'A';
        for (auto& st : textures) {
            std::string name = "test_texture_"s + ch + ".png";
        
            auto id = *atlas->add_subtexture(*ve::io::read_png(ve::io::paths::PATH_TILE_TEXTURES / name));
            st = *atlas->get_subtexture(id);
        
            ++ch;
        }
        
        
        // Create a mesh for the cube and add it to the pipeline.
        auto [vertices, indices] = ve::meshes::flat_indexed_textured_cube(
            std::array { textures[0], textures[0], textures[1], textures[1], textures[2],textures[2] },
            ve::vec3f { 0, 0, 1 }
        );
        
        ve::vertex_array<vertex> buffer{ std::span(vertices), std::span(indices) };
    
        pipeline->add_buffer(
            ve::shader_library::instance().get_or_load("texture_3d"s),
            std::make_shared<ve::vertex_array<vertex>>(std::move(buffer))
        );
        
        
        auto window = ve::window_manager::instance().get_window(game::window_id).lock();
        
    
        // Create a method to update the camera every frame.
        // The camera will rotate around the cube and slowly bob up and down.
        auto camera_fn = [size = window->get_canvas_size()](){
            static ve::perspective_camera camera;
        
            camera.set_aspect_ratio(size.x / size.y);
        
            static bool init = true;
            if (init) {
                camera.set_position(ve::vec3f{ 0, 0, -2 });
                camera.look_at({ 0, 0, 1 });
                init = false;
            }
        
            static float t = 0;
            t += 0.001;
            
            camera.move(camera.get_right() * 0.001f);
            camera.set_y(sin(t));
        
            camera.look_at(ve::vec3f{ 0, 0, 1 });
        
            return camera.get_matrix();
        };
        
    
        // Add the camera to the pipeline.
        pipeline->set_global_uniform_fn("camera"s, camera_fn);
        pipeline->set_global_uniform_val("tex"s, atlas->get_texture());
    
        // Create a layer in the layerstack for the pipeline to draw to.
        ve::layerstack_target tgt {
            "Cube Scene"s,
            ve::shared<ve::graphics_pipeline> { pipeline }
        };
    
        window->add_layer(std::move(tgt), 0.0);
    }
    
    
    void game::on_pre_loop(void) noexcept {
    
    }
    
    
    void game::on_post_loop(void) noexcept {
    
    }
    
    
    void game::on_pre_exit(void) noexcept {
    
    }
    
    
    void game::on_post_exit(void) noexcept {
    
    }
    
    
    [[nodiscard]] const ve::game_info* game::get_info(void) noexcept {
        static const ve::game_info info {
            .name         = "VoxelEngine Example Game",
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
}