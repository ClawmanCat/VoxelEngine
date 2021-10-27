#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/io/image.hpp>


namespace ve::gfx::missing_texture {
    // Purple / black checkerboard.
    inline const image_rgba8 color_texture = []{
        image_rgba8 result = filled_image({ 2, 2 }, colors::PURPLE);
        result[{0, 0}] = colors::BLACK;
        result[{1, 1}] = colors::BLACK;

        return result;
    }();

    // Default values for roughness, metalness, ao are zero.
    inline const image_rgba8 material_texture = filled_image({1, 1}, colors::BLACK);

    // Default value for normal textures is the forwards vector [0, 0, 1], i.e. pure blue.
    inline const image_rgba8 normal_texture = filled_image({1, 1}, colors::PURE_BLUE);

}