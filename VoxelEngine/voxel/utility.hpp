#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/direction.hpp>


namespace ve::voxel {
    // Returns the chunk a given position is in.
    constexpr static tilepos to_chunkpos(const tilepos& worldpos) {
        return worldpos >> tilepos::value_type(lsb(voxel_settings::chunk_size));
    }

    // Converts the position inside the given chunk to a position in the world.
    constexpr static tilepos to_worldpos(const tilepos& chunkpos, const tilepos& offset = tilepos { 0 }) {
        return (chunkpos << tilepos::value_type(lsb(voxel_settings::chunk_size))) + offset;
    }

    // Converts the given position to the offset within the chunk containing it.
    constexpr static tilepos to_localpos(const tilepos& worldpos) {
        return worldpos - to_worldpos(to_chunkpos(worldpos));
    }

    // Given an offset within a chunk that is out of bounds on one axis,
    // returns the direction of the neighbour containing that position.
    // E.g. given a chunk size of 32, [11, 32, 11] will return the "UP" position.
    constexpr static direction_t neighbour_direction(const tilepos& localpos) {
        return direction_from_vector(
            (-1 * vec3i(localpos < 0)) +
            vec3i(localpos >= voxel_settings::chunk_size)
        );
    }
}