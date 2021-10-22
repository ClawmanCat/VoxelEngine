#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/utility/direction.hpp>
#include <VoxelEngine/utility/cube.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(vertex/vertex_buffer.hpp)


// TODO: Templatize the methods in this file on the vertex & index types.
// TODO: Add per-face-texture support to textured_cube.
namespace ve::gfx {
    using textured_buffer_t = gfxapi::indexed_vertex_buffer<vertex_types::texture_vertex_3d, u32>;


    inline shared<textured_buffer_t> textured_quad(const vec2f& size, const subtexture& texture) {
        using vertex = vertex_types::texture_vertex_3d;


        constexpr std::array pos_deltas {
            vec3f { -0.5, -0.5, 0 },
            vec3f { -0.5, +0.5, 0 },
            vec3f { +0.5, -0.5, 0 },
            vec3f { +0.5, +0.5, 0 }
        };

        constexpr std::array uv_deltas {
            vec2f { 0, 1 },
            vec2f { 0, 0 },
            vec2f { 1, 1 },
            vec2f { 1, 0 }
        };


        std::vector<vertex> vertices;
        for (const auto& [pos, uv] : views::zip(pos_deltas, uv_deltas)) {
            vertices.emplace_back(vertex {
                .position      = pos * vec3f { size.x, size.y, 0 },
                .uv            = texture.uv + (texture.wh * uv),
                .texture_index = texture.binding
            });
        }

        std::vector<u32> indices = { 0, 2, 3, 0, 3, 1 };


        auto buffer = textured_buffer_t::create();
        buffer->store_vertices(vertices);
        buffer->store_indices(indices);

        return buffer;
    }


    inline shared<textured_buffer_t> textured_cube(const vec3f& size, const subtexture& texture) {
        using vertex = vertex_types::texture_vertex_3d;


        std::vector<vertex> vertices;
        std::vector<u32> indices;
        
        for (u8 direction = 0; direction < (u8) directions.size(); ++direction) {
            const auto& face_data = cube_face_data[direction];

            for (u8 i = 0; i < 4; ++i) {
                vertices.push_back(vertex {
                    .position         = face_data.positions[i] * size,
                    .uv               = texture.uv + (face_data.uvs[i] * texture.wh),
                    .texture_index    = texture.binding
                });
            }


            auto face_indices = cube_index_pattern;
            for (auto& idx : face_indices) idx = idx + (4 * direction);

            indices.insert(indices.end(), face_indices.begin(), face_indices.end());
        }


        auto buffer = textured_buffer_t::create();
        buffer->store_vertices(vertices);
        buffer->store_indices(indices);

        return buffer;
    }
}