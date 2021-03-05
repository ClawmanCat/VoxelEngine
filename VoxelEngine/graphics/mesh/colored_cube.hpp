#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/color.hpp>

#include <VoxelEngine/platform/graphics/graphics.hpp>
#include graphics_include(buffer/vertex.hpp)
#include graphics_include(buffer/indexed_vertex_buffer.hpp)

#include <array>


namespace ve::graphics {
    // Helper for matching cube faces to indices.
    struct cube_colors {
        color bottom, top, back, front, left, right;
        
        constexpr operator std::array<color, 6>(void) const {
            return { bottom, top, back, front, left, right };
        }
    };
    
    
    
    inline shared<buffer> colored_cube(const std::array<color, 6 * 4>& vertex_colors) {
        using vertex = color_vertex_3d;
    
        std::array axis_orderings {
            std::array { &vec3f::y, &vec3f::x, &vec3f::z },
            std::array { &vec3f::z, &vec3f::x, &vec3f::y },
            std::array { &vec3f::x, &vec3f::y, &vec3f::z }
        };
        
        constexpr std::array offsets = { -0.5f, 0.5f };
        std::array<vertex, 6 * 4> vertices;
        std::size_t index = 0;
        
        for (const auto& [first, second, third] : axis_orderings) {
            for (auto first_offset : offsets) {
                for (auto second_offset : offsets) {
                    for (auto third_offset : offsets) {
                        vec3f position;
                        position.*first  = first_offset;
                        position.*second = second_offset;
                        position.*third  = third_offset;
                        
                        vertices[index] = vertex {
                            .position = position,
                            .color    = vertex_colors[index]
                        };
                        
                        ++index;
                    }
                }
            }
        }
        
        constexpr std::array index_pattern = { 0, 2, 3, 0, 3, 1 };
        std::array<u32, 6 * 6> indices;
        index = 0;
        
        for (std::size_t side = 0; side < 6; ++side) {
            for (auto vert_index : index_pattern) {
                indices[index++] = (4 * side) + vert_index;
            }
        }
        
        
        return std::make_shared<indexed_vertex_buffer<vertex>>(
            std::span { vertices },
            std::span { indices  }
        );
    }
    
    
    inline shared<graphics::buffer> colored_cube(const color& cube_color) {
        std::array<color, 6 * 4> vertex_colors;
        vertex_colors.fill(cube_color);
        
        return colored_cube(vertex_colors);
    }
    
    
    inline shared<buffer> colored_cube(const std::array<color, 6>& side_colors) {
        std::array<color, 6 * 4> vertex_colors;
        for (std::size_t side = 0; side < 6; ++side) {
            std::fill(
                vertex_colors.begin() + (4 * side),
                vertex_colors.begin() + (4 * side) + 4,
                side_colors[side]
            );
        }
        
        return colored_cube(vertex_colors);
    }
}