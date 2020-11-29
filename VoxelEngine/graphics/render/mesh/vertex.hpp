#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex_attribute.hpp>

#include <cstddef>
#include <array>


namespace ve::vertex {
    // Vertex types for scenes without lighting.
    namespace flat {
        template <std::size_t N> struct color_vertex {
            vec<N, float> position;
            vec4ub color;
            
            VE_GEN_ATTRIB_FN(color_vertex, position, color)
        };
    
        using color_vertex_2d = color_vertex<2>;
        using color_vertex_3d = color_vertex<3>;
    
    
        template <std::size_t N> struct texture_vertex {
            vec<N, float> position;
            vec2us uv;
    
            VE_GEN_ATTRIB_FN(texture_vertex, position, uv)
        };
    
        using texture_vertex_2d = texture_vertex<2>;
        using texture_vertex_3d = texture_vertex<3>;
    }
}