#include <VEDemoGame/game.hpp>

#include <VoxelEngine/graphics/render/buffer/vertex_array.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex.hpp>
#include <VoxelEngine/graphics/render/mesh/meshes.hpp>
#include <VoxelEngine/graphics/render/camera/perspective_camera.hpp>
#include <VoxelEngine/graphics/render/shader/shader_library.hpp>
#include <VoxelEngine/graphics/render/target/layerstack_target.hpp>
#include <VoxelEngine/graphics/render/graphics_pipeline.hpp>
#include <VoxelEngine/graphics/layerstack.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/utils/color.hpp>
#include <VoxelEngine/utils/logger.hpp>


namespace demo_game {
    void game::on_pre_init(void) noexcept {
        VE_LOG_INFO("This message was produced in game code.");
    }
   
    
    void game::on_post_init(void) noexcept {
        // Below is example code to render a cube to the screen.
        using vertex = ve::vertex::flat::color_vertex_3d;
        using colors = ve::colors;
        
        // Create a pipeline to render the cube with.
        auto pipeline = std::make_unique<ve::graphics_pipeline>();
    
        // Create a mesh for the cube and add it to the pipeline.
        auto [vertices, indices] = ve::meshes::flat_indexed_colored_cube(
            std::array { ve::colors::ORANGE, colors::GREEN, colors::YELLOW, colors::BLACK, colors::BLUE, colors::BROWN },
            ve::vec3f { 0, 0, 1 }
        );
    
        ve::vertex_array<vertex> buffer { std::span(vertices), std::span(indices) };
    
        pipeline->add_buffer(
            ve::shader_library::instance().get_or_load("flat_color_3d"s),
            std::make_unique<ve::vertex_array<vertex>>(std::move(buffer))
        );
        
    
        // Create a method to update the camera every frame.
        // The camera will rotate around the cube and slowly bob up and down.
        auto camera_fn = [](){
            static ve::perspective_camera camera;
        
            auto size = ve::window_manager::instance().get_canvas_size();
            camera.set_aspect_ratio(size.x / size.y);
        
            static bool init = true;
            if (init) {
                camera.set_position(ve::vec3f{ 0, 0, -1 });
                camera.look_at({ 0, 0, 1 });
                init = false;
            }
        
            static float t = 0;
            t += 0.0001;
            
            camera.move(camera.get_right() * 0.001f);
            camera.set_y(sin(t));
        
            camera.look_at(ve::vec3f{ 0, 0, 1 });
        
            return camera.get_matrix();
        };
        
    
        // Add the camera to the pipeline.
        pipeline->set_global_uniform_fn("camera"s, camera_fn);
    
        // Create a layer in the layerstack for the pipeline to draw to.
        ve::layerstack_target tgt {
            "Cube Scene"s,
            std::move(pipeline)
        };
    
        ve::layerstack::instance().add_layer(std::move(tgt), 0.0);
    }
    
    
    void game::on_pre_loop(void) noexcept {
    
    }
    
    
    void game::on_post_loop(void) noexcept {
    
    }
    
    
    void game::on_pre_exit(void) noexcept {
    
    }
    
    
    void game::on_post_exit(void) noexcept {
    
    }
    
    
    void game::on_actor_id_provided(ve::actor_id id) noexcept {
        game::id = id;
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