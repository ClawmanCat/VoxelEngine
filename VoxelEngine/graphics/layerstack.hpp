#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/target/layerstack_target.hpp>

#include <utility>


namespace ve {
    class layerstack {
    public:
        [[nodiscard]] static layerstack& instance(void) noexcept {
            static layerstack i {};
            return i;
        }
        
        
        void draw(void) {
            for (auto& [index, target] : layers) target.draw();
        }
        
        
        // Higher z-indices are rendered later. (And thus appear above lower ones.)
        void add_layer(layerstack_target&& target, float z_index) {
            layers[z_index] = std::move(target);
        }
        
        
        bool remove_layer(std::string_view name) {
            auto it = std::find_if(
                layers.begin(),
                layers.end(),
                [&](const auto& kv) { return kv.second.get_name() == name; }
            );
            
            if (it != layers.end()) {
                layers.erase(it);
                return true;
            }
            
            return false;
        }
    private:
        tree_map<float, layerstack_target> layers;
    };
}