#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/direction.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex.hpp>

#include <array>


namespace ve::meshes {
    template <typename Index = u32>
    constexpr auto flat_indexed_colored_cube(
        std::array<vec4ub, 6> colors,
        vec3f position = { 0, 0, 0 },
        vec3f size = { 1, 1, 1 }
    ) {
        using vertex = vertex::flat::color_vertex_3d;
        
        return std::pair {
            // TODO: Store this in a model file.
            std::array<vertex, 6 * 4> {{
                // Bottom
                { .position = position + (vec3f { -0.5, -0.5, +0.5 } * size), .color = colors[0] },
                { .position = position + (vec3f { +0.5, -0.5, +0.5 } * size), .color = colors[0] },
                { .position = position + (vec3f { -0.5, -0.5, -0.5 } * size), .color = colors[0] },
                { .position = position + (vec3f { +0.5, -0.5, -0.5 } * size), .color = colors[0] },
                // Top
                { .position = position + (vec3f { -0.5, +0.5, +0.5 } * size), .color = colors[1] },
                { .position = position + (vec3f { +0.5, +0.5, +0.5 } * size), .color = colors[1] },
                { .position = position + (vec3f { -0.5, +0.5, -0.5 } * size), .color = colors[1] },
                { .position = position + (vec3f { +0.5, +0.5, -0.5 } * size), .color = colors[1] },
                // Front
                { .position = position + (vec3f { -0.5, +0.5, -0.5 } * size), .color = colors[2] },
                { .position = position + (vec3f { +0.5, +0.5, -0.5 } * size), .color = colors[2] },
                { .position = position + (vec3f { -0.5, -0.5, -0.5 } * size), .color = colors[2] },
                { .position = position + (vec3f { +0.5, -0.5, -0.5 } * size), .color = colors[2] },
                // Back
                { .position = position + (vec3f { -0.5, +0.5, +0.5 } * size), .color = colors[3] },
                { .position = position + (vec3f { +0.5, +0.5, +0.5 } * size), .color = colors[3] },
                { .position = position + (vec3f { -0.5, -0.5, +0.5 } * size), .color = colors[3] },
                { .position = position + (vec3f { +0.5, -0.5, +0.5 } * size), .color = colors[3] },
                // Left
                { .position = position + (vec3f { -0.5, +0.5, +0.5 } * size), .color = colors[4] },
                { .position = position + (vec3f { -0.5, +0.5, -0.5 } * size), .color = colors[4] },
                { .position = position + (vec3f { -0.5, -0.5, +0.5 } * size), .color = colors[4] },
                { .position = position + (vec3f { -0.5, -0.5, -0.5 } * size), .color = colors[4] },
                // Right
                { .position = position + (vec3f { +0.5, +0.5, +0.5 } * size), .color = colors[5] },
                { .position = position + (vec3f { +0.5, +0.5, -0.5 } * size), .color = colors[5] },
                { .position = position + (vec3f { +0.5, -0.5, +0.5 } * size), .color = colors[5] },
                { .position = position + (vec3f { +0.5, -0.5, -0.5 } * size), .color = colors[5] }
            }},
            
            std::array<Index, 6 * 6> {
                (4 * 0) + 2, (4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2, (4 * 0) + 1, (4 * 0) + 3,
                (4 * 1) + 2, (4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2, (4 * 1) + 1, (4 * 1) + 3,
                (4 * 2) + 2, (4 * 2) + 0, (4 * 2) + 1, (4 * 2) + 2, (4 * 2) + 1, (4 * 2) + 3,
                (4 * 3) + 2, (4 * 3) + 0, (4 * 3) + 1, (4 * 3) + 2, (4 * 3) + 1, (4 * 3) + 3,
                (4 * 4) + 2, (4 * 4) + 0, (4 * 4) + 1, (4 * 4) + 2, (4 * 4) + 1, (4 * 4) + 3,
                (4 * 5) + 2, (4 * 5) + 0, (4 * 5) + 1, (4 * 5) + 2, (4 * 5) + 1, (4 * 5) + 3
            }
        };
    }
}