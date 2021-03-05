#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/layerstack_target.hpp>


namespace ve::graphics {
    class layerstack : public resource_owner<layerstack> {
    public:
        // Make sure the float used to request removal of a layer is actually identical
        // to the one that was used to insert it.
        // This prevents unexpected behaviour due to floating point precision errors.
        class layer_id {
        public:
            explicit layer_id(float z_index) : z_index(z_index) {}
            ve_comparable(layer_id);
        private:
            friend class layerstack;
            float z_index;
        };
        
        
        void on_actor_destroyed(actor_id id) {
            remove_all_owned_from_vector(
                id,
                layers,
                ve_get_field(second.first),
                ve_tf_field(kv, "at Z = "s + std::to_string(kv.first.z_index)),
                "layerstack target"
            );
        }
        
        
        void draw(void) {
            for (auto& [z_index, target] : layers) target.second.draw();
        }
        
        
        // Adds a layer to the layerstack.
        // If a layer already exists at the given z-index, the new layer is inserted above the old one.
        layer_id add_layer(layerstack_target&& target, float z_index, ve_default_actor(actor_id)) {
            while (layers.contains(layer_id { z_index })) {
                VE_LOG_WARN("Layer "s + std::to_string(z_index) + " is already taken. Trying next layer.");
                z_index = std::nextafter(z_index, std::numeric_limits<float>::max());
            }
            
            layer_id id { z_index };
            layers.insert(std::pair { id, std::pair { actor_id, std::move(target) } });
            return id;
        }
        
        void remove_layer(layer_id id) {
            layers.erase(id);
        }
        
        optional<layerstack_target&> get_layer(layer_id id) {
            auto it = layers.find(id);
            
            if (it == layers.end()) return nullopt;
            else return it->second.second;
        }
        
        optional<layerstack_target&> get_nth_layer(std::size_t n) {
            auto it = layers.nth(n);
    
            if (it == layers.end()) return nullopt;
            else return it->second.second;
        }
        
    private:
        small_flat_map<layer_id, std::pair<actor_id, layerstack_target>> layers;
    };
}