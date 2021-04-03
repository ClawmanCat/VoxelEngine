#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    class voxel_space;
    
    
    class chunk_loader {
    public:
        explicit chunk_loader(voxel_space* space) : space(space) {}
        virtual ~chunk_loader(void) = default;
        
    protected:
        void load_chunk(const vec3i& where);
        void unload_chunk(const vec3i& where);
        
        voxel_space* space;
    };
}