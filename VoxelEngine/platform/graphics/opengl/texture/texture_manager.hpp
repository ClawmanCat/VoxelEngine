#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_atlas.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/generative_texture_atlas.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/utility/io/resource.hpp>


namespace ve::graphics {
    template <typename Atlas = aligned_generative_texture_atlas<>>
    class texture_manager : public resource_owner<texture_manager<Atlas>> {
    public:
        void on_actor_destroyed(actor_id id) {
            auto it = texture_owners.find(id);
            if (it == texture_owners.end()) return;
            
            for (const auto& res : it->second) {
                textures.erase(res);
            }
            
            texture_owners.erase(it);
        }
        
        
        expected<subtexture> get_texture(io::static_resource res, ve_default_actor(owner)) {
            auto it = textures.find(res);
            if (it != textures.end()) return it->second;
            
            
            auto image = io::read_png((fs::path) res);
            if (!image) return make_unexpected(image.error());
            
            auto tex = atlas.add_texture(*image, owner);
            if (!tex) return tex;
            
            
            textures.insert({ res, *tex });
            texture_owners[owner].push_back(res);
            
            
            return tex;
        }
    private:
        Atlas atlas;
        
        hash_map<io::static_resource, subtexture> textures;
        hash_map<actor_id, std::vector<io::static_resource>> texture_owners;
    };
}