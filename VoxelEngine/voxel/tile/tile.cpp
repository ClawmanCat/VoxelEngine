#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/graphics/texture/texture_source.hpp>
#include <VoxelEngine/graphics/texture/utility/utility.hpp>
#include <VoxelEngine/graphics/texture/missing_texture.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/cube.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


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
        transparent(args.transparent),
        solid(args.solid)
    {}


    // Given a set of visible sides of this tile, appends vertices for those sides to the given mesh.
    // visible_sides is a bitfield where bit n corresponds to ve::directions[n].
    void tile::append_mesh(tile_mesh& dest, u8 visible_sides, tile_metadata_t meta) const {
        if (!rendered) return;

        direction_t rendered_directions = 0;
        for (direction_t direction = 0; direction < (direction_t) directions.size(); ++direction) {
            if (visible_sides & (1 << direction)) {
                static auto& texmgr = voxel_settings::get_texture_manager();


                gfx::file_image_source color_texture_src {
                    &get_texture_for_side(direction, gfx::texture_type::COLOR_TEXTURE, meta),
                    &gfx::missing_texture::color_texture
                };

                gfx::file_image_source normal_texture_src {
                    &get_texture_for_side(direction, gfx::texture_type::NORMAL_TEXTURE, meta),
                    &gfx::missing_texture::normal_texture
                };


                constexpr std::array material_textures {
                    gfx::texture_type::ROUGHNESS_TEXTURE,
                    gfx::texture_type::METALNESS_TEXTURE,
                    gfx::texture_type::AMBIENT_OCCLUSION_TEXTURE,
                    gfx::texture_type::EMISSIVE_TEXTURE
                };

                auto paths = material_textures
                    | views::transform([&](const auto& tex) { return fs::weakly_canonical(get_texture_for_side(direction, tex, meta)); })
                    | ranges::to<std::vector>;


                // Generate a new material texture from the roughness, metalness, ao and emissive maps if one doesn't exist already.
                gfx::generative_image_source material_texture_src {
                    cat_range_with(paths, "|"),
                    [&] {
                        // Foreach component map, generate a texture source.
                        auto sources = create_filled_array<material_textures.size()>([&] (std::size_t i) {
                            return gfx::file_image_source { &paths[i], &gfx::missing_texture::material_texture };
                        });


                        // And acquire the image pointers.
                        auto ptrs = create_filled_array<material_textures.size()>(produce((const image_rgba8*) nullptr));

                        raii_function release_images { no_op, [&] {
                            for (const auto& [i, ptr] : ptrs | views::enumerate) {
                                if (ptr) sources[i].relinquish(ptr);
                            }
                        } };

                        for (const auto& [i, src] : sources | views::enumerate) ptrs[i] = src.require();


                        // And generate the texture.
                        return gfx::make_material_texture(*ptrs[0], *ptrs[1], *ptrs[2], *ptrs[3]);
                    }
                };


                auto tex_color    = texmgr.get_or_load(color_texture_src);
                auto tex_normal   = texmgr.get_or_load(normal_texture_src);
                auto tex_material = texmgr.get_or_load(material_texture_src);


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

                    auto indices = cube_index_pattern;
                    for (auto& index : indices) index += (4 * rendered_directions);

                    dest.indices.insert(dest.indices.end(), indices.begin(), indices.end());
                } else {
                    dest.vertices.reserve(dest.vertices.size() + cube_index_pattern.size());
                    for (u32 i : cube_index_pattern) dest.vertices.push_back(vertices[i]);
                }


                ++rendered_directions;
            }
        }
    }
}