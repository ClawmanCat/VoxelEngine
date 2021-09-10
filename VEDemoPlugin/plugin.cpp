#include <VEDemoPlugin/plugin.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/string.hpp>


namespace demo_plugin {
    const ve::plugin_info* plugin::get_info(void) {
        const static ve::plugin_info info {
            .display_name  = "Demo Plugin",
            .internal_name = "clawmancat.demoplugin",
            .description   = { "A simple plugin to demonstrate basic functionality." },
            .authors       = { "ClawmanCat" },
            .version       = { 0, 0, 1, "PreAlpha" }
        };
        
        return &info;
    }
    
    
    void plugin::on_loaded(ve::actor_id id, bool dynamic) {
        plugin::id = id;
        VE_LOG_INFO(ve::cat("plugin::on_loaded in ", plugin::get_info()->internal_name));
    }
    
    
    void plugin::on_unloaded(bool dynamic) {
    
    }
}


void on_loaded(ve::actor_id id, bool dynamic) {
    demo_plugin::plugin::on_loaded(id, dynamic);
}

void on_unloaded(bool dynamic) {
    demo_plugin::plugin::on_unloaded(dynamic);
}

const ve::plugin_info* get_info(void) {
    return demo_plugin::plugin::get_info();
}