#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>


namespace ve::voxel::detail {
    tile_registry& default_get_tile_registry(void) {
        static tile_registry i { };
        return i;
    }


    gfx::texture_manager<>& default_get_texture_manager(void) {
        static gfx::texture_manager i { };
        return i;
    }


    gfx::mesh_types::material_mesh::vertex_t default_assemble_vertex(const vertex_assembler_arguments& args) {
        return gfx::mesh_types::material_mesh::vertex_t {
            .position      = args.position,
            .normal        = args.normal,
            .tangent       = args.tangent,
            .uv_color      = args.color_texture.uv    + (args.uv * args.color_texture.wh),
            .uv_normal     = args.normal_texture.uv   + (args.uv * args.normal_texture.wh),
            .uv_material   = args.material_texture.uv + (args.uv * args.material_texture.wh),
            .texture_index = args.color_texture.binding
        };
    }


    std::array<const tile*, 2> default_get_skip_tile_list(void) {
        return { tiles::TILE_AIR, tiles::TILE_UNKNOWN };
    }
}