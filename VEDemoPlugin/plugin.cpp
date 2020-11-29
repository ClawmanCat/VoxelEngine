#include <VEDemoPlugin/plugin.hpp>
#include <VoxelEngine/utils/logger.hpp>


namespace demo_plugin {
    void plugin::on_loaded(ve::actor_id id) noexcept {
        plugin::id = id;
        
        VE_LOG_INFO("This message was produced in plugin code.");
        // TODO: Add example code for plugin interactions.
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