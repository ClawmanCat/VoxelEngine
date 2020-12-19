#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/graphics/render/target/layerstack_target.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>

#include <utility>


namespace ve {
    class layerstack : public resource_owner<layerstack> {
    public:
        void on_actor_destroyed(ve::actor_id id) {
            std::erase_if(
                layers,
                [&](const auto& pair) {
                    const auto& [z_index, target] = pair;
                    
                    if (target.first == id) {
                        VE_LOG_DEBUG("Removing layerstack target "s + target.second.get_name() + " because its owner was destroyed.");
                        return true;
                    }
                    
                    return false;
                }
            );
        }
        
        
        void draw(void) {
            for (auto& [index, target] : layers) target.second.draw();
        }
        
        
        // Higher z-indices are rendered later. (And thus appear above lower ones.)
        void VE_RESOURCE_FN(add_layer, layerstack_target&& target, float z_index) {
            layers[z_index] = { owner, std::move(target) };
        }
        
        
        bool remove_layer(std::string_view name) {
            auto it = std::find_if(
                layers.begin(),
                layers.end(),
                [&](const auto& kv) { return kv.second.second.get_name() == name; }
            );
            
            if (it != layers.end()) {
                layers.erase(it);
                return true;
            }
            
            return false;
        }
    private:
        tree_map<float, std::pair<actor_id, layerstack_target>> layers;
    };
}