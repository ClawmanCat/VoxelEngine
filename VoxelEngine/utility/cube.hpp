#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve {
    constexpr inline std::array<u32, 6> cube_index_pattern = { 0, 2, 3, 0, 3, 1 };


    // Manual initialization of vector fields is required here since GLM uses unions internally,
    // so one of the fields must be activated to be used in a constexpr context.
    struct cube_face {
        std::array<vec3f, 4> positions = create_filled_array<4>(produce(vec3f { 0 }));
        std::array<vec2f, 4> uvs = create_filled_array<4>(produce(vec2f { 0 }));
        vec3f normal = vec3f { 0 };
        vec3f tangent = vec3f { 0 };
        u32 face_index = 0;
    };


    // Provides a standardized (vertex / index order, tangents, etc.) way of iterating over cube faces.
    // This method is used when polygonizing voxel data.
    template <typename Pred> requires std::is_invocable_v<Pred, const cube_face&>
    constexpr inline void foreach_cube_face(Pred pred) {
        // Order is chosen such that iteration occurs in the same order that faces appear in ve::direction.
        constexpr std::array axis_orderings {
            std::array { &vec3f::z, &vec3f::x, &vec3f::y },
            std::array { &vec3f::y, &vec3f::x, &vec3f::z },
            std::array { &vec3f::x, &vec3f::z, &vec3f::y }
        };

        constexpr std::array uv_orderings {
            vec2f { 0, 0 }, vec2f { 0, 1 }, vec2f { 1, 0 }, vec2f { 1, 1 }
        };

        constexpr std::array offsets = { 0.5f, -0.5f };


        std::size_t cube_vertex_index = 0;
        std::size_t cube_face_index = 0;
        for (const auto& [first, second, third] : axis_orderings) {
            for (auto first_offset : offsets) {
                cube_face face_data;

                face_data.normal = vec3f { 0 };
                face_data.normal.*first = 2.0f * first_offset;

                face_data.face_index = (u32) cube_face_index;


                std::size_t face_vertex_index = 0;
                for (auto second_offset : offsets) {
                    for (auto third_offset : offsets) {
                        // Vertex position.
                        vec3f& position  = face_data.positions[face_vertex_index];
                        position.*first  = first_offset;
                        position.*second = second_offset;
                        position.*third  = third_offset;


                        // Vertex UV.
                        // Note: back (0) and left(5) sides have flipped UVs.
                        std::size_t face_uv_index = face_vertex_index;
                        if (in_range(cube_vertex_index, 0u * 4, 1u * 4) || in_range(cube_vertex_index, 5u * 4, 6u * 4)) {
                            face_uv_index = (face_uv_index + 2) % 4;
                        }

                        face_data.uvs[face_vertex_index] = uv_orderings[face_uv_index];


                        ++cube_vertex_index;
                        ++face_vertex_index;
                    }
                }


                // Tangent can be calculated from the normal and the direction of increasing UV.
                vec3f e1  = face_data.positions[2] - face_data.positions[1];
                vec3f e2  = face_data.positions[3] - face_data.positions[1];
                vec2f uv1 = face_data.uvs[2] - face_data.uvs[1];
                vec2f uv2 = face_data.uvs[3] - face_data.uvs[1];

                float f = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);
                face_data.tangent = uv2.y * e1 - uv1.y * e2;


                std::invoke(pred, face_data);
                ++cube_face_index;
            }
        }
    }


    constexpr inline std::array<cube_face, 6> cube_face_data = [] {
        std::array<cube_face, 6> result;
        foreach_cube_face([&] (const auto& face) { result[face.face_index] = face; });
        return result;
    }();
}