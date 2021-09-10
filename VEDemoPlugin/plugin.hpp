#pragma once

#include <VEDemoPlugin/core/core.hpp>
#include <VoxelEngine/dependent/dependent_info.hpp>
#include <VoxelEngine/dependent/actor.hpp>


namespace demo_plugin {
    class plugin {
    public:
        plugin(void) = delete;
        
        static const ve::plugin_info* get_info(void);
        
        static void on_loaded(ve::actor_id id, bool dynamic);
        static void on_unloaded(bool dynamic);
    
        VE_GET_STATIC_VAL(id);
    private:
        static inline ve::actor_id id = ve::invalid_actor_id;
    };
}


ve_plugin_api void on_loaded(ve::actor_id id, bool dynamic);
ve_plugin_api void on_unloaded(bool dynamic);
ve_plugin_api const ve::plugin_info* get_info(void);