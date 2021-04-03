#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/graphics/mesh/voxel_mesher_helpers.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace ve {
    tile::tile(tile_parameters&& params) :
        name(std::move(params.name)),
        colliding(params.colliding),
        rendered(params.rendered),
        num_states(params.num_states),
        owner(params.owner),
        texture_path(io::paths::PATH_TILE_TEXTURES / (name + ".png"))
    {
        tile_registry::instance().register_tile(this);
    }
    
    
    tile::~tile(void) {
        if (id != invalid_tile) tile_registry::instance().unregister_tile(this);
    }
    
    
    tile& tile::operator=(tile&& o) {
        std::swap(name,          o.name);
        std::swap(colliding,     o.colliding);
        std::swap(rendered,      o.rendered);
        std::swap(num_states,    o.num_states);
        std::swap(owner,         o.owner);
        std::swap(id,            o.id);
        std::swap(default_state, o.default_state);
        std::swap(texture_path,  o.texture_path);
        
        return *this;
    }
    
    
    void tile::mesh(voxel_mesh_t& allocated_mesh, tile_metadata state, direction occluded_sides) const {
        std::size_t sides_meshed = 0;
        
        for (std::size_t i = 0; i < num_directions; ++i) {
            direction dir = direction(1 << i);
            // TODO: Fix occlusion mapping!
            // if (bool(occluded_sides & dir)) continue;
            
            
            // TODO: Use missing texture if texture cannot be found.
            auto tex = *voxel_settings::tile_texture_manager.get_texture(
                get_texture_for_side(dir, state)
            );
            
            
            for (std::size_t j = 0; j < 4; ++j) {
                allocated_mesh.vertices.emplace_back(
                    voxel_settings::tile_vertex_fn(
                        this,
                        tex,
                        graphics::cube_faces[i].positions[j],
                        tex.uv + (graphics::cube_faces[i].uvs[j] * tex.size)
                    )
                );
            }
            
            
            if constexpr (voxel_mesh_t::indexed) {
                auto indices = graphics::cube_index_pattern;
                for (auto& index : indices) index += (sides_meshed * 4);
                
                allocated_mesh.indices.insert(
                    allocated_mesh.indices.end(),
                    indices.begin(),
                    indices.end()
                );
            }
            
            
            ++sides_meshed;
        }
    }
    
    
    bool tile::occludes_side(direction dir) const {
        return this->is_rendered();
    }
    
    
    const fs::path& tile::get_texture_for_side(direction dir, tile_metadata state) const {
        return get_texture(state);
    }
    
    
    const fs::path& tile::get_texture(tile_metadata state) const {
        return texture_path;
    }
}