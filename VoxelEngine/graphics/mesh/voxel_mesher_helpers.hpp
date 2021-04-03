#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/utility.hpp>

#include <type_traits>
#include <array>


namespace ve::graphics {
    constexpr inline std::array<u32, 6> cube_index_pattern = { 0, 2, 3, 0, 3, 1 };
    
    
    // Calls pred(positions, uvs) for every face of the cube in the order specified in directions.
    template <typename Pred> requires std::is_invocable_v<Pred, const std::array<vec3f, 4>&, const std::array<vec2f, 4>&>
    constexpr inline void enumerate_cube_faces(Pred pred) {
        constexpr std::array axis_orderings {
            std::array { &vec3f::y, &vec3f::x, &vec3f::z },
            std::array { &vec3f::z, &vec3f::x, &vec3f::y },
            std::array { &vec3f::x, &vec3f::z, &vec3f::y }
        };
        
        constexpr std::array uv_orderings {
            vec2f { 1, 1 }, vec2f { 1, 0 }, vec2f { 0, 1 }, vec2f { 0, 0 }
        };
    
        constexpr std::array offsets = { 0.0f, 1.0f };
        
        
        std::size_t vertex_index = 0;
        for (const auto& [first, second, third] : axis_orderings) {
            for (auto first_offset : offsets) {
                auto positions = filled_array<vec3f, 4>(vec3f { 0 });
                auto uvs = filled_array<vec2f, 4>(vec2f { 0 });
                
                std::size_t face_index = 0;
                for (auto second_offset : offsets) {
                    for (auto third_offset : offsets) {
                        vec3f& position = positions[face_index];
                        position.*first  = first_offset;
                        position.*second = second_offset;
                        position.*third  = third_offset;
                        
                        // Front (3) and left (4) sides have flipped UVs.
                        std::size_t uv_face = face_index;
                        if (in<std::size_t>(vertex_index, 3u * 4, 5u * 4)) uv_face = (uv_face + 2) % 4;
                        
                        uvs[face_index] = uv_orderings[uv_face];
                        
                        
                        ++vertex_index;
                        ++face_index;
                    }
                }
                
                pred(positions, uvs);
            }
        }
    }
    
    
    // Calls pred(indices) for every face of the cube in the order specified in directions.
    template <typename Pred> requires std::is_invocable_v<Pred, const std::array<u32, 6>&>
    constexpr inline void enumerate_cube_face_indices(Pred pred) {
        std::array<u32, 6> indices = cube_index_pattern;
        for (std::size_t side = 0; side < 6; ++side) {
            pred(indices);
            for (auto& index : indices) index += 6;
        }
    }
    
    
    struct cube_face_info {
        std::array<vec3f, 4> positions;
        std::array<vec2f, 4> uvs;
        std::array<u32,   6> indices;
    };
    
    constexpr inline auto cube_faces = []() {
        std::array<cube_face_info, 6> cube_faces;
        
        enumerate_cube_faces([&, i = 0](const auto& pos, const auto& uv) mutable {
            cube_faces[i].positions = pos;
            cube_faces[i].uvs = uv;
            ++i;
        });
        
        enumerate_cube_face_indices([&, i = 0](const auto& indices) mutable {
            cube_faces[i++].indices = indices;
        });
        
        return cube_faces;
    }();
}