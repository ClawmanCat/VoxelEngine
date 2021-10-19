#include <VoxelEngine/voxel/tile/tile.hpp>


namespace ve::voxel {
    tile::tile(const arguments& args) :
        name(args.name),
        textures(create_filled_array<magic_enum::enum_count<gfx::texture_type>()>([&] (std::size_t i) {
            return io::paths::PATH_TILE_TEXTURES / cat(
                args.texture_name,
                "_",
                gfx::texture_type_name(magic_enum::enum_value<gfx::texture_type>(i)),
                ".png"
            );
        })),
        num_states(args.num_states),
        rendered(args.rendered),
        transparent(args.transparent)
    {}


    // Given a set of visible sides of this tile, appends vertices for those sides to the given mesh.
    // visible_sides is a bitfield where bit n corresponds to ve::directions[n].
    void tile::append_mesh(tile_mesh& dest, u8 visible_sides, tile_metadata_t meta) {
        if (!rendered) return;

        for (u8 direction = 0; direction < (u8) directions.size(); ++direction) {
            if (visible_sides & (1 << direction)) {
                static auto& texmgr = voxel_settings::get_texture_manager();


                auto tex_color  = texmgr.get_or_load(get_texture_for_side(direction, gfx::texture_type::COLOR_TEXTURE, meta));
                auto tex_normal = texmgr.get_or_load(get_texture_for_side(direction, gfx::texture_type::NORMAL_TEXTURE, meta));

                // For the material texture merge roughness, metalness and occlusion textures into a single RGB texture.
                const auto& path_roughness = fs::canonical(get_texture_for_side(direction, gfx::texture_type::ROUGHNESS_TEXTURE, meta));
                const auto& path_metalness = fs::canonical(get_texture_for_side(direction, gfx::texture_type::METALNESS_TEXTURE, meta));
                const auto& path_occlusion = fs::canonical(get_texture_for_side(direction, gfx::texture_type::AMBIENT_OCCLUSION_TEXTURE, meta));

                auto tex_material = texmgr.get_or_load(
                    // TODO: Use a better way to identify resources.
                    cat(path_roughness, "|", path_metalness, "|", path_occlusion),
                    [&] {
                        return gfx::make_material_texture(
                            io::load_image(path_roughness),
                            io::load_image(path_metalness),
                            io::load_image(path_occlusion)
                        );
                    }
                );


                const auto& face_data = cube_face_data[direction];

                auto vertices = create_filled_array<4>([&] (std::size_t i) {
                    return voxel_settings::assemble_vertex(vertex_assembler_arguments {
                        .tile             = this,
                        .color_texture    = tex_color,
                        .normal_texture   = tex_normal,
                        .material_texture = tex_material,
                        .position         = face_data.positions[i],
                        .normal           = face_data.normal,
                        .tangent          = face_data.tangent,
                        .uv               = face_data.uvs[i]
                    });
                });


                if constexpr (tile_mesh::indexed) {
                    dest.vertices.insert(dest.vertices.end(), vertices.begin(), vertices.end());
                    dest.indices.insert(dest.indices.end(), cube_index_pattern.begin(), cube_index_pattern.end());
                } else {
                    dest.vertices.reserve(dest.vertices.size() + cube_index_pattern.size());

                    for (u32 i : cube_index_pattern) {
                        dest.vertices.push_back(vertices[i]);
                    }
                }
            }
        }
    }
}