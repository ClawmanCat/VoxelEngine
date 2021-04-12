#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/voxel/space.hpp>


namespace ve {
    void chunk_loader::load_chunk(const vec3i& where) { space->load_chunk(where); }
    void chunk_loader::unload_chunk(const vec3i& where) { space->unload_chunk(where); }
}