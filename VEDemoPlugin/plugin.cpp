#include <VEDemoPlugin/plugin.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/graphics/window.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex.hpp>
#include <VoxelEngine/graphics/render/texture/aligned_texture_atlas.hpp>
#include <VoxelEngine/graphics/render/mesh/meshes.hpp>
#include <VoxelEngine/graphics/render/shader/shader_library.hpp>
#include <VoxelEngine/graphics/render/camera/perspective_camera.hpp>


namespace demo_plugin {
    void plugin::on_loaded(ve::actor_id id) noexcept {
        plugin::id = id;
    
        // Below is example code to render a cube to the screen.
        using vertex = ve::vertex::flat::texture_vertex_3d;
    
    
        VE_LOG_DEBUG("Creating window for "s + plugin::get_info()->name + ".");
        auto window = ve::window_manager::instance()
            .create_window({ .title = plugin::get_info()->name.c_str(), .maximized = false }, id)
            .lock();
        
        
        // Initialize texture atlas.
        // TODO: Store atlases as resource_owner.
        static ve::aligned_texture_atlas atlas { };
    
        // Create a pipeline to render the cube with.
        auto pipeline = std::make_shared<ve::graphics_pipeline>();
    
        // Load texture.
        auto tex_id  = *atlas.add_subtexture(*ve::io::read_png(ve::io::paths::PATH_TILE_TEXTURES / "missing_texture.png"));
        auto texture = *atlas.get_subtexture(tex_id);
        
    
        // Create a mesh for the cube and add it to the pipeline.
        auto [vertices, indices] = ve::meshes::flat_indexed_textured_cube(
            std::array { texture, texture, texture, texture, texture, texture },
            ve::vec3f { 0, 0, 1 }
        );
    
        ve::vertex_array<vertex> buffer { std::span(vertices), std::span(indices) };
    
        pipeline->add_buffer(
            ve::shader_library::instance().get_or_load("texture_3d"s),
            std::make_shared<ve::vertex_array<vertex>>(std::move(buffer))
        );
        
    
        // Create a method to update the camera every frame.
        // The camera will rotate around the cube and slowly bob up and down.
        auto camera_fn = [size = window->get_canvas_size()](){
            static ve::perspective_camera camera;
            camera.set_aspect_ratio(size.x / size.y);
        
            static bool init = true;
            if (init) {
                camera.set_position(ve::vec3f{ 0, 0, -1 });
                camera.look_at({ 0, 0, 1 });
                init = false;
            }
        
            return camera.get_matrix();
        };
    
    
        // Add the camera to the pipeline.
        pipeline->set_global_uniform_fn("camera"s, camera_fn);
        pipeline->set_global_uniform_val("tex"s, atlas.get_texture());
    
        // Create a layer in the layerstack for the pipeline to draw to.
        ve::layerstack_target tgt {
            "Plugin Cube Scene"s,
            std::move(pipeline)
        };
    
        window->add_layer(std::move(tgt), 0.0, id);
    }
    
    
    void plugin::on_unloaded(ve::actor_id id) noexcept {
        // TODO: Add example code for plugin unloading.
    }
    
    
    const ve::plugin_info* plugin::get_info(void) noexcept {
        static const ve::plugin_info info {
            .name           = "VoxelEngine Example Plugin",
            .internal_name  = "clawmancat.ve_example_plugin",
            .description    = { "Example plugin to demonstrate basic functionality." },
            .authors        = { "ClawmanCat" },
            .plugin_version = {
                "PreAlpha",
                VEDEMOPLUGIN_VERSION_MAJOR,
                VEDEMOPLUGIN_VERSION_MINOR,
                VEDEMOPLUGIN_VERSION_PATCH
            },
            
            // Control whether the plugin can be loaded / unloaded while the engine is running, or only at startup / shutdown.
            .allow_dynamic_load   = true,
            .allow_dynamic_unload = true
        };
        
        return &info;
    }
}