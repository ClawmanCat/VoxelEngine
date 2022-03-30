#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler/compiler.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/layout.hpp>


namespace ve::gfx { class shader_cache; }


namespace ve::gfx::opengl {
    class shader;


    extern shared<shader> make_shader(
        const shader_compilation_data& data,
        shader_cache* cache,
        const ctti::type_id_t& vertex_type,
        std::span<const vertex_attribute> vertex_layout,
        const detail::vertex_layout& vertex_layout_reflection
    );


    template <typename Vertex> inline shared<shader> make_shader(const shader_compilation_data& data, shader_cache* cache) {
        auto layout = std::span<const vertex_attribute> { Vertex::get_vertex_layout().begin(), Vertex::get_vertex_layout().end() };
        return make_shader(data, cache, ctti::type_id<Vertex>(), layout, detail::generate_vertex_layout<Vertex>(data.reflection.get_input_stage().second));
    }
}