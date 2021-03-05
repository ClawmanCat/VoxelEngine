#pragma once

#include <VEDemoPlugin/core/core.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/dependent/plugin.hpp>


namespace demo_plugin {
    class plugin {
    public:
        static void on_loaded(ve::actor_id id) {
            plugin::id = id;
        }
        
        
        static void on_unloaded(void) {}
        
        
        [[nodiscard]] static const ve::plugin_info* get_info(void) {
            static const ve::plugin_info info {
                .name                 = "VoxelEngine Demo Plugin",
                .internal_name        = "meloncat.demo_plugin",
                .description          = { "Example plugin to demonstrate basic functionality." },
                .authors              = { "ClawmanCat" },
                .plugin_version       = {
                    "PreAlpha",
                    VEDEMOGAME_VERSION_MAJOR,
                    VEDEMOGAME_VERSION_MINOR,
                    VEDEMOGAME_VERSION_PATCH
                },
                .allow_dynamic_load   = true,
                .allow_dynamic_unload = true
            };
            
            return &info;
        }
        
        
        static ve::actor_id get_id(void) { return plugin::id; }
    
    private:
        static inline ve::actor_id id = ve::no_actor_id;
    };
}


// Callbacks for the game engine.
plugin_api const ve::plugin_info* get_plugin_info(void) {
    return demo_plugin::plugin::get_info();
}

plugin_api void on_plugin_load(ve::actor_id id) {
    demo_plugin::plugin::on_loaded(id);
}

plugin_api void on_plugin_unload(void) {
    demo_plugin::plugin::on_unloaded();
}