#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>


namespace ve::gfx::opengl {
    // 6 empty vertices used to render a quad in g_input shaders.
    inline shared<vertex_buffer> g_input_quad(void) {
        static auto i = [] {
            auto result = unindexed_vertex_buffer<vertex_types::no_vertex>::create();
            result->store_vertices(std::vector(6, vertex_types::no_vertex { }));

            return result;
        } ();

        return i;
    };


    // Create a new pipeline_draw_data, equivalent to the provided one but using the g_input_quad as VBO.
    inline pipeline_draw_data draw_data_to_g_input(const pipeline_draw_data& data) {
        return pipeline_draw_data {
            .buffers         = { g_input_quad().get() },
            .ctx             = data.ctx
        };
    }
}