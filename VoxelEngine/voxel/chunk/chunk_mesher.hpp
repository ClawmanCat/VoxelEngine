#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>
#include <VoxelEngine/utility/direction.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>


namespace ve::voxel {
    namespace detail {
        struct mesh_cache_key {
            tile_data data;
            direction_t visible_sides;

            ve_eq_comparable(mesh_cache_key);
            ve_hashable();
        };


        // Keeps track of a list of tiledatas that are not rendered and can be skipped when encountered.
        struct skip_tile_list {
            constexpr static inline std::size_t skip_count = meta::array_size<
                std::remove_cvref_t<decltype(voxel_settings::get_skip_tile_list())>
            >;


            skip_tile_list(void) :
                skip_data(create_filled_array<skip_count>([] (std::size_t i) {
                    return voxel_settings::get_tile_registry().get_default_state(voxel_settings::get_skip_tile_list()[i]);
                }))
            {
                for (const tile* t : voxel_settings::get_skip_tile_list()) {
                    VE_ASSERT(t->is_stateless(), "Only stateless tiles may be added to the mesher skip list.");
                    VE_ASSERT(!t->is_rendered(), "Only tiles that are not rendered may be added to the mesher skip list.");
                    VE_ASSERT(!voxel_settings::get_tile_registry().is_removable(t), "Mesher skip list tiles must be marked non-removable in the tile registry.");
                }
            }


            bool skip(const tile_data& td) const {
                return ranges::contains(skip_data, td);
            }


            std::array<tile_data, skip_count> skip_data;
        };
    }


    inline tile_mesh mesh_chunk(const voxel_space* space, const chunk* chunk, const tilepos& chunkpos) {
        static thread_local hash_map<detail::mesh_cache_key, tile_mesh> mesh_cache { };
        static detail::skip_tile_list skip_list { };


        // Converts normalized tile coordinates to coordinates within the mesh.
        auto denormalize = [](tile_mesh& mesh, const tilepos& pos, std::size_t vertex_offset, std::size_t index_offset) {
            for (auto& vertex : ranges::subrange(mesh.vertices.begin() + vertex_offset, mesh.vertices.end())) {
                vertex.position += pos;
            }

            if constexpr (tile_mesh::indexed) {
                for (auto& index : ranges::subrange(mesh.indices.begin() + index_offset, mesh.indices.end())) {
                    index += vertex_offset;
                }
            }
        };


        tile_mesh result;
        chunk->foreach([&](const auto& where, const auto& data) {
            if (skip_list.skip(data)) return;


            const tile* tile = voxel_settings::get_tile_registry().get_tile_for_state(data);
            tile_metadata_t meta = voxel_settings::get_tile_registry().get_effective_metastate(data);

            if (!tile->is_rendered()) return;


            // Find sides that need to be rendered.
            auto is_visible = [&](const tilepos& where, direction_t side) {
                tilepos neighbour = where + tilepos { directions[side] };

                tile_data neighbour_data;
                if (glm::any(neighbour < 0 || neighbour >= voxel_settings::chunk_size)) {
                    neighbour_data = space->get_data(chunkpos * tilepos { voxel_settings::chunk_size } + neighbour);
                } else {
                    neighbour_data = chunk->get_data(neighbour);
                }


                // If the neighbour is invisible, we need to render this side.
                if (skip_list.skip(neighbour_data)) return true;

                const class tile* neighbour_tile = voxel_settings::get_tile_registry().get_tile_for_state(neighbour_data);
                tile_metadata_t neighbour_meta = voxel_settings::get_tile_registry().get_effective_metastate(neighbour_data);

                // If this side is occluded by the neighbour, we don't have to render it.
                return !(neighbour_tile->occludes_side(opposing_direction(side), neighbour_meta));
            };


            // Push the given mesh to the back of the chunk mesh and denormalize it.
            auto push_and_denormalize = [&](const auto& mesh, const tilepos& pos) {
                std::size_t vertex_start = result.vertices.size();
                result.vertices.insert(
                    result.vertices.end(),
                    mesh.vertices.begin(),
                    mesh.vertices.end()
                );

                std::size_t index_start = 0;
                if constexpr (tile_mesh::indexed) {
                    index_start = result.indices.size();

                    result.indices.insert(
                        result.indices.end(),
                        mesh.indices.begin(),
                        mesh.indices.end()
                    );
                }

                denormalize(result, where, vertex_start, index_start);
            };


            u8 visible_sides = 0;
            for (direction_t dir = 0; dir < directions.size(); ++dir) {
                visible_sides |= (u8(is_visible(where, dir)) << dir);
            }


            // If we've meshed this state before, just re-use the old value.
            detail::mesh_cache_key key {
                .data = data,
                .visible_sides = visible_sides
            };


            if (auto it = mesh_cache.find(key); it != mesh_cache.end()) {
                push_and_denormalize(it->second, where);
            } else {
                // Even though a heap allocation still needs to happen when the mesh is copied into the cache,
                // this can still save several heap allocations as the mesh is pushed back into the buffer.
                static thread_local tile_mesh temporary_storage { };
                temporary_storage.clear();

                tile->append_mesh(temporary_storage, visible_sides, meta);
                push_and_denormalize(temporary_storage, where);

                mesh_cache.emplace(key, std::move(temporary_storage));
            }
        });


        return result;
    }
}