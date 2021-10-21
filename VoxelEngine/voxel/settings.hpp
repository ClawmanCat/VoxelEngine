#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/mesh.hpp>
#include <VoxelEngine/graphics/texture/texture_manager.hpp>

#include <bit>


namespace ve::voxel {
    class tile;
    class tile_registry;


    struct vertex_assembler_arguments {
        const tile* tile;
        const gfx::subtexture& color_texture, normal_texture, material_texture;

        ve::vec3f position;
        ve::vec3f normal;
        ve::vec3f tangent;
        ve::vec2f uv;
    };


    namespace detail {
        extern tile_registry& default_get_tile_registry(void);
        extern gfx::texture_manager<>& default_get_texture_manager(void);
        extern gfx::mesh_types::material_mesh::vertex_t default_assemble_vertex(const vertex_assembler_arguments& args);
        extern const std::array<const tile*, 2>& default_get_skip_tile_list(void);
    }


    template <typename Base = meta::null_type> struct voxel_settings_t {
        // General Voxel Settings
        using tile_position_t = vec3i;

        // Note: this value MUST be a power of two.
        constexpr static std::size_t chunk_size = 32;


        // Tile Storage Settings
        using tile_id_t = u16;
        using tile_metadata_t = u16;

        // Since a lot of tiles will typically not have any metadata, it is wasteful to assign storage for that metadata.
        // Instead a few tile ids can be marked as stateless storage, meaning their metadata can be used to store different tiles instead.
        constexpr static std::size_t reserved_stateless_tile_ids = 4;

        static tile_registry& get_tile_registry(void) {
            return detail::default_get_tile_registry();
        }


        // Voxel Meshing Settings
        using tile_mesh_t     = gfx::mesh_types::material_mesh;
        using texture_atlas_t = gfx::aligned_generative_texture_atlas;

        // This is the texture atlas that will be used to store textures for tiles.
        static gfx::texture_manager<texture_atlas_t>& get_texture_manager(void) {
            return detail::default_get_texture_manager();
        }

        // This method is used to construct vertices for the given mesh type.
        static tile_mesh_t::vertex_t assemble_vertex(const vertex_assembler_arguments& args) {
            return detail::default_assemble_vertex(args);
        }

        // The tile mesher performs an early check for these tiles, so rendering them can be aborted early.
        // Tiles in this list must be non-rendered, be stateless and non-removable from the registry.
        static const auto& get_skip_tile_list(void) {
            return detail::default_get_skip_tile_list();
        }
    };


    // Allow game to provide its own version of this struct.
    #ifdef VE_OVERLOADED_VOXEL_SETTINGS
        using voxel_settings = VE_OVERLOADED_VOXEL_SETTINGS<voxel_settings_t<>>;
    #else
        using voxel_settings = voxel_settings_t<void>;
    #endif


    using tilepos         = typename voxel_settings::tile_position_t;
    using tile_mesh       = typename voxel_settings::tile_mesh_t;
    using tile_id_t       = typename voxel_settings::tile_id_t;
    using tile_metadata_t = typename voxel_settings::tile_metadata_t;


    static_assert(std::popcount(voxel_settings::chunk_size) == 1, "Chunk size must be a power of two.");
    static_assert(meta::glm_traits<tilepos>::is_vector, "Tile position type must be a vector.");
    static_assert(std::is_signed_v<tilepos::value_type> && std::is_integral_v<tilepos::value_type>, "Tile position element type must be a signed integer.");
    static_assert(std::is_unsigned_v<tile_id_t>, "Tile ID type must be an unsigned integer.");
    static_assert(std::is_unsigned_v<tile_metadata_t>, "Tile metadata type must be an unsigned integer.");
}