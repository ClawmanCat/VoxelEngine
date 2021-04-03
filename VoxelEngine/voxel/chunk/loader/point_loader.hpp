#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/distance.hpp>


namespace ve {
    template <auto DistanceFn = distance_functions::euclidean<chunkpos>{}>
    class point_loader : public chunk_loader {
    public:
        point_loader(voxel_space* space, const vec3i& chunk, const vec3i& radius = vec3i{0}) :
            chunk_loader(space), chunk(chunk), radius(radius)
        {
            spatial_iterate(chunk, radius, [&](const vec3i& where, std::size_t dim) {
                if (DistanceFn.within(chunk, where, radius[dim])) {
                    load_chunk(where);
                    VE_LOG_DEBUG(glm::to_string(where));
                }
            });
        }
        
        
        ~point_loader(void) {
            spatial_iterate(chunk, radius, [&](const vec3i& where, std::size_t dim) {
                if (DistanceFn.within(chunk, where, radius[dim])) unload_chunk(where);
            });
        }
    
    private:
        vec3i chunk;
        vec3i radius;
    };
}
