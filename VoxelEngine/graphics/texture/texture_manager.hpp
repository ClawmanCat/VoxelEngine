#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>
#include <VoxelEngine/graphics/texture/generative_texture_atlas.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utility/io/io.hpp>


namespace ve::graphics {
    template <typename Atlas = aligned_generative_texture_atlas<>>
    class texture_manager : public resource_owner<texture_manager<Atlas>> {
    public:
        template <typename... Args>
        explicit texture_manager(Args&&... args) : atlas(std::forward<Args>(args)...) {}
        
        
        void on_actor_destroyed(actor_id id) {
            auto it = texture_owners.find(id);
            if (it == texture_owners.end()) return;
            
            for (const auto& path : it->second) {
                textures.erase(path);
            }
            
            texture_owners.erase(it);
        }
        
        
        expected<subtexture> get_texture(const fs::path& path, ve_default_actor(owner)) {
            auto it = textures.find(path);
            if (it != textures.end()) return it->second;
            
            
            auto image = io::read_png(path);
            if (!image) return make_unexpected(image.error());
            
            auto tex = atlas.add_texture(*image, owner);
            if (!tex) return tex;
            
            
            textures.insert({ path, *tex });
            texture_owners[owner].push_back(path);
            
            
            return tex;
        }
    private:
        Atlas atlas;
        
        hash_map<fs::path, subtexture> textures;
        hash_map<actor_id, std::vector<fs::path>> texture_owners;
    };
}