#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>
#include <VoxelEngine/utility/direction.hpp>


namespace ve::graphics {
    namespace detail {
        struct mesh_cache_key {
            tile_data data;
            direction occluded;
        
            ve_hashable(data, occluded);
            ve_eq_comparable(mesh_cache_key);
        };
    }
    
    
    inline voxel_mesh_t mesh_chunk(const chunk& chnk) {
        // Empty tiles are very likely to be common, so optimize the mesher to skip these.
        const static tile_data void_type_td = tile_registry::instance().get_default_tile_state(tiles::TILE_VOID);
        // For a given combination of tile data and occluded directions, the tile should always produce the same mesh,
        // so we can cache the result.
        static hash_map<detail::mesh_cache_key, voxel_mesh_t> mesh_cache;
        
        
        // Converts normalized mesh coordinates to coordinates within the chunk.
        auto denormalize_mesh = [](voxel_mesh_t& mesh, const vec3i& pos, std::size_t vert_start, std::size_t idx_start) {
            for (auto& vert : ranges::subrange(mesh.vertices.begin() + vert_start, mesh.vertices.end())) {
                vert.position += pos;
            }
            
            if constexpr (voxel_mesh_t::indexed) {
                for (auto& idx : ranges::subrange(mesh.indices.begin() + idx_start, mesh.indices.end())) {
                    idx += vert_start;
                }
            }
        };
        
        
        voxel_mesh_t result;
        chnk.foreach([&](const vec3i& pos, u32 index, const tile_data& data) {
            // Don't render air.
            // (This could be handled by the next statement, but this is such a common case it is worth eliminating the lookup.)
            if (data == void_type_td) return;
            
            
            // Don't render invisible tiles.
            const tile* tile = tile_registry::instance().get_tile(data);
            if (!tile->is_rendered()) return;
            
            
            // Find occluded sides.
            auto occludes = [&](const vec3i& pos, direction dir) {
                const vec3i neighbour = pos + direction_vector(dir);
                
                // Assume positions outside the chunk to not be occluded.
                // TODO: Change this.
                if (glm::any(neighbour < 0 || neighbour >= (i32) voxel_settings::chunk_size)) {
                    return false;
                }
                
                const auto& td = chnk[neighbour];
                if (td == void_type_td) return false;
                
                return true;
                
                const auto* t = tile_registry::instance().get_tile(td);
                return t->occludes_side(opposing(dir));
            };
            
            direction occluded = direction::NONE;
            for (auto dir : direction{}) {
                if (occludes(pos, dir)) occluded |= dir;
            }
            
            
            // Either get cached mesh or make a new one.
            detail::mesh_cache_key key {
                .data = data,
                .occluded = occluded
            };
    
            if (auto it = mesh_cache.find(key); it == mesh_cache.end()) {
                // Keep some allocated space for repeated mesh generation.
                // Unless the new mesh is larger than all previously generated ones,
                // this reduces the number of heap allocations to at most 1 (to copy the mesh into the result).
                static thread_local voxel_mesh_t mesh_storage;
                tile->mesh(mesh_storage, data.metadata, occluded);
                
                
                mesh_cache.emplace(key, mesh_storage);
                
                result.vertices.insert(
                    result.vertices.end(),
                    mesh_storage.vertices.begin(),
                    mesh_storage.vertices.end()
                );
                
                std::size_t idx_start = 0;
                if constexpr (voxel_mesh_t::indexed) {
                    idx_start = result.indices.size();
    
                    result.indices.insert(
                        result.indices.end(),
                        mesh_storage.indices.begin(),
                        mesh_storage.indices.end()
                    );
                }
                
                
                denormalize_mesh(
                    result,
                    pos,
                    result.vertices.size() - mesh_storage.vertices.size(),
                    idx_start
                );
                
                
                // Clear for next iteration (Allocation is preserved).
                mesh_storage.vertices.clear();
                if constexpr (voxel_mesh_t::indexed) mesh_storage.indices.clear();
            } else {
                auto& cached_mesh = it->second;
                
                
                result.vertices.insert(
                    result.vertices.end(),
                    cached_mesh.vertices.begin(),
                    cached_mesh.vertices.end()
                );
                
                std::size_t idx_start = 0;
                if constexpr (voxel_mesh_t::indexed) {
                    idx_start = result.indices.size();
                    
                    result.indices.insert(
                        result.indices.end(),
                        cached_mesh.indices.begin(),
                        cached_mesh.indices.end()
                    );
                }
                
                
                denormalize_mesh(
                    result,
                    pos,
                    result.vertices.size() - cached_mesh.vertices.size(),
                    idx_start
                );
            }
        });
        
        
        return result;
    }
}