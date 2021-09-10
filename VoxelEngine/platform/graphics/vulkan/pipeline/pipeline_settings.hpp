#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct pipeline_settings {
        enum class topology_t {
            POINTS         = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            LINES          = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            TRIANGLES      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            TRIANGLE_FAN   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            TRIANGLE_STRIP = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            PATCHES        = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
        } topology = topology_t::TRIANGLES;


        enum class polygon_mode_t {
            SURFACE  = VK_POLYGON_MODE_FILL,
            EDGES    = VK_POLYGON_MODE_LINE,
            VERTICES = VK_POLYGON_MODE_POINT
        } polygon_mode = polygon_mode_t::SURFACE;


        enum class cull_mode_t {
            NO_CULLING = VK_CULL_MODE_NONE,
            CULL_FRONT = VK_CULL_MODE_FRONT_BIT,
            CULL_BACK  = VK_CULL_MODE_BACK_BIT,
            CULL_BOTH  = VK_CULL_MODE_FRONT_AND_BACK
        } cull_mode = cull_mode_t::CULL_BACK;


        enum class cull_direction_t {
            CLOCKWISE         = VK_FRONT_FACE_CLOCKWISE,
            COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE
        } cull_direction = cull_direction_t::CLOCKWISE;


        float line_width   = 1.0f;
        bool  enable_depth = true;
        bool  clamp_depth  = true;
    };
}