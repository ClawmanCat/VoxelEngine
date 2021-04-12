#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/distance.hpp>


namespace ve {
    template <auto DistanceFn = distance_functions::euclidean<chunkpos>{}>
    class point_loader : public chunk_loader {
    public:
        point_loader(voxel_space* space, const vec3i& chunk, const u32 radius = 0) :
            chunk_loader(space), chunk(chunk), radius(radius)
        {
            space->toggle_mesh_updates(false);
            
            spatial_iterate(chunk, vec3i { (i32) radius }, [&](const vec3i& where) {
                if (DistanceFn.within(chunk, where, (i32) radius)) load_chunk(where);
            });
    
            space->toggle_mesh_updates(true);
        }
        
        
        ~point_loader(void) {
            space->toggle_mesh_updates(false);
            
            spatial_iterate(chunk, vec3i { (i32) radius }, [&](const vec3i& where) {
                if (DistanceFn.within(chunk, where, (i32) radius)) unload_chunk(where);
            });
    
            space->toggle_mesh_updates(true);
        }
    
    private:
        vec3i chunk;
        u32 radius;
    };
}
