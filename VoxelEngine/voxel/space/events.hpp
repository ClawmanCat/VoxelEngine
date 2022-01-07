#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>


namespace ve::voxel {
    class voxel_space;


    struct space_update_event {
        voxel_space* space;
        nanoseconds dt;
    };


    struct chunk_generated_event {
        voxel_space* space;
        tilepos chunkpos;
    };


    struct chunk_loaded_event {
        voxel_space* space;
        tilepos chunkpos;
        std::size_t current_load_count;
    };


    struct chunk_unloaded_event {
        voxel_space* space;
        tilepos chunkpos;
        std::size_t current_load_count;
    };


    struct voxel_changed_event {
        voxel_space* space;
        tilepos where;
        tile_data old_value;
        tile_data new_value;
    };


    struct chunk_remeshed_event {
        voxel_space* space;
        tilepos chunkpos;
    };
}