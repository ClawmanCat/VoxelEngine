#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/attribute.hpp>

#include <vector>
#include <type_traits>


namespace ve::graphics {
    namespace detail {
        template <typename Derived> class vertex_requirements {
            void fn_wrapper(void) {
                static_assert(std::is_trivial_v<Derived>, "Vertex types must be trivial.");
            }
        };
    }
    
    
    struct color_vertex_2d : detail::vertex_requirements<color_vertex_2d> {
        vec2f position;
        vec4ub color;
        
        VE_GEN_ATTRIB_FN(color_vertex_2d, position, color);
    };
    
    struct color_vertex_3d : detail::vertex_requirements<color_vertex_3d> {
        vec3f position;
        vec4ub color;
        
        VE_GEN_ATTRIB_FN(color_vertex_3d, position, color);
    };
    
    
    struct flat_texture_vertex_2d : detail::vertex_requirements<flat_texture_vertex_2d> {
        vec2f position;
        vec2f uv;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(flat_texture_vertex_2d, position, uv, tex_index);
    };
    
    struct flat_texture_vertex_3d : detail::vertex_requirements<flat_texture_vertex_3d> {
        vec3f position;
        vec2f uv;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(flat_texture_vertex_3d, position, uv, tex_index);
    };
    
    
    struct phong_texture_vertex_3d : detail::vertex_requirements<phong_texture_vertex_3d> {
        vec3f position;
        vec2f uv;
        vec3f normal;
        u8 material;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(phong_texture_vertex_3d, position, uv, normal, material, tex_index);
    };
    
    
    template <typename Vertex> struct unindexed_mesh {
        std::vector<Vertex> vertices;
        
        using vertex_t = Vertex;
        constexpr static inline bool indexed = false;
    };
    
    using unindexed_color_mesh_2d   = unindexed_mesh<color_vertex_2d>;
    using unindexed_color_mesh      = unindexed_mesh<color_vertex_3d>;
    using unindexed_texture_mesh_2d = unindexed_mesh<flat_texture_vertex_2d>;
    using unindexed_texture_mesh    = unindexed_mesh<flat_texture_vertex_3d>;
    using unindexed_phong_mesh      = unindexed_mesh<phong_texture_vertex_3d>;
    
    
    template <typename Vertex, typename Index = u32> struct indexed_mesh {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        
        using vertex_t = Vertex;
        using index_t  = Index;
        constexpr static inline bool indexed = true;
    };
    
    using color_mesh_2d   = indexed_mesh<color_vertex_2d>;
    using color_mesh      = indexed_mesh<color_vertex_3d>;
    using texture_mesh_2d = indexed_mesh<flat_texture_vertex_2d>;
    using texture_mesh    = indexed_mesh<flat_texture_vertex_3d>;
    using phong_mesh      = indexed_mesh<phong_texture_vertex_3d>;
}