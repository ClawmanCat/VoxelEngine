#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>

#include <ctti/nameof.hpp>


namespace ve::gfx {
    // Assures the provided vertex type can be used for the shader with the provided inputs.
    template <typename Vertex> requires has_vertex_layout<Vertex>
    inline void validate_vertex_layout(const std::vector<reflect::attribute>& inputs) {
        const auto& layout = Vertex::get_vertex_layout();

        for (const auto& input : inputs) {
            auto it = ranges::find_if(layout, equal_on(&vertex_attribute::name, input.name));

            if (it == layout.end()) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " not fulfilled by vertex type ", ctti::nameof<Vertex>().cppstring()
                ));
            }

            if (!it->type.can_initialize_vertex_attribute(input.type)) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " has a type not compatible with that provided by vertex type ", ctti::nameof<Vertex>().cppstring()
                ));
            }
        }
    }
}