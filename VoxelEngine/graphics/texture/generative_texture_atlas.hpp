#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>
#include <VoxelEngine/graphics/texture/aligned_texture_atlas.hpp>


namespace ve::graphics {
    // A wrapper around a set of texture atlases.
    // If one atlas becomes full, a new one is automatically generated and used instead.
    template <typename Atlas> requires std::is_base_of_v<texture_atlas<Atlas>, Atlas>
    class generative_texture_atlas : public texture_atlas<generative_texture_atlas<Atlas>> {
    public:
        // Args are used to generate new atlases.
        template <typename... Args>
        explicit generative_texture_atlas(Args&&... args) {
            generator = [...args = std::forward<Args>(args)]() {
                return Atlas { args... };
            };
        }
        
        
        ve_move_only(generative_texture_atlas);
    
    
        void on_actor_destroyed(actor_id id) {
            // Some textures may have been deleted, reset the largest known available size.
            for (auto& [atlas, max_size] : storage) max_size = tex_size(size());
        }
    
    
        expected<subtexture> add_texture(const io::image& img, ve_default_actor(owner)) {
            VE_ASSERT(
                glm::all(img.size <= size()),
                "Cannot insert texture into atlas that is larger than the atlas itself."
            );
            
            // Try inserting the texture into an existing atlas.
            std::size_t index = 0;
            
            for (auto& [atlas, max_size] : storage) {
                if (tex_size(img.size) <= max_size) {
                    auto tex = atlas.add_texture(img, owner);
                    
                    if (tex) {
                        texture_containers[tex->tex->get_id()] = index;
                        return tex;
                    } else {
                        VE_LOG_DEBUG("Atlas "s + std::to_string(index) + " has no space for texture. Trying next atlas.");
                        max_size = degrade_size(max_size);
                    }
                }
                
                ++index;
            }
            
            // Create a new atlas for the texture.
            VE_LOG_DEBUG(
                "Creating new texture atlas since no atlas is available to store texture of size "s +
                "[" + std::to_string(img.size.x) + ", " + std::to_string(img.size.y) + "]."
            );
            
            storage.push_back({ generator(), tex_size(size()) });
            
            auto st = storage.back().atlas.add_texture(img, owner);
            if (st) st->index = storage.size() - 1;
            return st;
        }
    
    
        void remove_texture(const subtexture& tex) {
            auto it = texture_containers.find(tex.tex->get_id());
            if (it == texture_containers.end()) return;
            
            storage[it->second].remove_texture(tex);
            texture_containers.erase(it);
        }
    
    
        static u32 quantization(void) {
            return Atlas::quantization();
        }
    
    
        static vec2ui size(void) {
            return Atlas::size();
        }
    private:
        struct atlas_storage {
            Atlas atlas;
            u32 largest_possible_texture;
        };
    
        std::function<Atlas(void)> generator;
        std::vector<atlas_storage> storage;
        hash_map<GLuint, std::size_t> texture_containers;
        
        
        static u32 tex_size(const vec2ui& size) {
            return std::min(size.x, size.y);
        }
        
        static u32 degrade_size(u32 size) {
            if (quantization() >= size) return 0;
            else return size - quantization();
        }
    };
    
    
    template <std::size_t W = 4096, std::size_t H = 4096, std::size_t Align = 32>
    using aligned_generative_texture_atlas = generative_texture_atlas<aligned_texture_atlas<W, H, Align>>;
}