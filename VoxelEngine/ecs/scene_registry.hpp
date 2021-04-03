#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/side/side.hpp>


namespace ve {
    template <side Side> class scene;
    
    
    template <side Side> class scene_registry : public resource_owner<scene_registry<Side>> {
    public:
        static scene_registry& instance(void);
        
        void on_actor_destroyed(actor_id id);
        
        void add_scene(scene<Side>* s);
        void remove_scene(scene<Side>* s);
        void update_scenes(microseconds dt);
    private:
        flat_map<actor_id, flat_set<scene<Side>*>> scenes;
    };
}