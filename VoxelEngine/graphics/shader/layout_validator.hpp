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
        return validate_vertex_layout(ctti::nameof<Vertex>().cppstring(), Vertex::get_vertex_layout(), inputs);
    }


    inline void validate_vertex_layout(std::string_view name, const auto& layout, const std::vector<reflect::attribute>& inputs) {
        for (const auto& input : inputs) {
            auto it = ranges::find_if(layout, [&](const auto& attrib) { return attrib.name == input.name; });

            if (it == layout.end()) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " not fulfilled by vertex type ", name
                ));
            }

            if (!it->type.can_initialize_vertex_attribute(input.type)) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " has a type not compatible with that provided by vertex type ", name
                ));
            }
        }
    }
}