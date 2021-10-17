#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(vertex/vertex_buffer.hpp)


namespace ve::gfx {
    using textured_buffer_t = gfxapi::indexed_vertex_buffer<vertex_types::texture_vertex_3d, u32>;


    inline shared<textured_buffer_t> textured_quad(const vec2f& size, const subtexture& texture) {
        using vertex = vertex_types::texture_vertex_3d;


        constexpr std::array pos_deltas {
            ve::vec3f { -0.5, -0.5, 0 },
            ve::vec3f { -0.5, +0.5, 0 },
            ve::vec3f { +0.5, -0.5, 0 },
            ve::vec3f { +0.5, +0.5, 0 }
        };

        constexpr std::array uv_deltas {
            ve::vec2f { 0, 1 },
            ve::vec2f { 0, 0 },
            ve::vec2f { 1, 1 },
            ve::vec2f { 1, 0 }
        };


        std::vector<vertex> vertices;
        for (const auto& [pos, uv] : ve::views::zip(pos_deltas, uv_deltas)) {
            vertices.emplace_back(vertex {
                .position      = pos * vec3f { size.x, size.y, 0 },
                .uv            = texture.uv + (texture.wh * uv),
                .texture_index = texture.binding
            });
        }

        std::vector<ve::u32> indices = { 0, 2, 3, 0, 3, 1 };


        auto buffer = textured_buffer_t::create();
        buffer->store_vertices(vertices);
        buffer->store_indices(indices);


        return buffer;
    }
}