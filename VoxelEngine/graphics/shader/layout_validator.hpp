#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/spirtype.hpp>

#include <ctti/nameof.hpp>


namespace ve::gfx {
    // Assures the provided vertex type can be used for the shader with the provided inputs.
    template <typename Vertex> requires has_vertex_layout<Vertex>
    inline void validate_vertex_layout(const std::vector<reflect::attribute>& inputs) {
        constexpr auto layout = Vertex::get_vertex_layout();

        for (const auto& input : inputs) {
            auto it = ranges::find_if(layout, equal_on(&vertex_attribute::name, input.name));

            if (it == layout.end()) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " not fulfilled by vertex type ", ctti::nameof<Vertex>().cppstring()
                ));
            }

            if (reflect::compare_spirtypes(input.type, reflect::spirtype_for<Vertex>()) != std::strong_ordering::equal) {
                throw std::runtime_error(cat(
                    "Shader input ", input.name,
                    " has a type not compatible with that provided by vertex type ", ctti::nameof<Vertex>().cppstring()
                ));
            }
        }
    }
}