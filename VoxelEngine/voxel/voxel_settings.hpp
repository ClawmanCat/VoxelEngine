#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/overloadable_setting.hpp>
#include <VoxelEngine/graphics/texture/texture_manager.hpp>

#include <type_traits>


namespace ve {
    // To change these settings, overload this class for the type ve::overloaded_settings_tag.
    // You can use the macro VE_OVERLOAD_SETTINGS for this purpose.
    template <typename = void> struct overloadable_voxel_settings {
        // Dimensions of a chunk in number of blocks.
        // If this is not a power of two, voxel operations will probably be significantly slower.
        constexpr static inline std::size_t chunk_size = 32;
        
        // Number of tile IDs in the tile registry allocated for tiles that do not have any metadata.
        // This can be used to reduce tile ID usage, by mapping many tiles to the same ID but with a different state.
        constexpr static inline std::size_t tile_registry_stateless_alloc = 32;
        
        // Underlying storage types used for tile IDs and tile metadata.
        using tile_id_t       = u16;
        using tile_metadata_t = u16;
        
        // Types used for positions.
        using chunkpos = vec3i;
        using tilepos  = vec3i;
        using worldpos = vec3f;
        
        // Tile texture size to optimize for. Other sizes are still allowed.
        constexpr static inline std::size_t optimal_tile_texture_size = 32;
        
        // Tile atlas dimensions.
        constexpr static inline std::size_t tile_atlas_size = 4096;
        
        // Storage object used for tile textures.
        static inline graphics::texture_manager<
            graphics::aligned_generative_texture_atlas<
                tile_atlas_size,
                tile_atlas_size,
                optimal_tile_texture_size
            >
        > tile_texture_manager { };
        
        // Type used for voxel meshes.
        using mesh_t = graphics::texture_mesh;
        
        // Function used to construct a vertex for such a mesh.
        constexpr static auto tile_vertex_fn = [](auto* tile, graphics::subtexture tex, const vec3f& pos, const vec2f& uv) {
            return graphics::flat_texture_vertex_3d {
                .position  = pos,
                .uv        = uv,
                .tex_index = tex.index
            };
        };
    };
    
    
    using voxel_settings = overloadable_voxel_settings<overloaded_settings_tag>;
    
    using chunkpos     = typename voxel_settings::chunkpos;
    using tilepos      = typename voxel_settings::tilepos;
    using worldpos     = typename voxel_settings::worldpos;
    using voxel_mesh_t = typename voxel_settings::mesh_t;
}