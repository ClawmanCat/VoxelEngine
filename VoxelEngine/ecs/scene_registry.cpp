#include <VoxelEngine/ecs/scene_registry.hpp>
#include <VoxelEngine/ecs/scene.hpp>


namespace ve {
    template <side Side> scene_registry<Side>& scene_registry<Side>::instance() {
        static scene_registry<Side> instance;
        return instance;
    }
    
    
    template <side Side> void scene_registry<Side>::on_actor_destroyed(actor_id id) {
        scenes.erase(id);
    }


    template <side Side> void scene_registry<Side>::add_scene(scene<Side>* s) {
        scenes[s->get_owner()].insert(s);
    }


    template <side Side> void scene_registry<Side>::remove_scene(scene<Side>* s) {
        scenes[s->get_owner()].erase(s);
    }


    template <side Side> void scene_registry<Side>::update_scenes(microseconds dt) {
        for (auto& [owner, scenes_for_owner] : scenes) {
            for (auto& scene : scenes_for_owner) {
                scene->update(dt);
            }
        }
    }
    
    
    template class scene_registry<side::CLIENT>;
    template class scene_registry<side::SERVER>;
}