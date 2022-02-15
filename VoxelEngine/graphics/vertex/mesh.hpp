#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>


namespace ve::gfx::mesh_types {
    namespace vt = ve::gfx::vertex_types;


    template <typename Vertex> struct unindexed_mesh {
        std::vector<Vertex> vertices;

        using vertex_t = Vertex;
        constexpr static inline bool indexed = false;


        void clear(void) {
            vertices.clear();
        }
    };

    template <typename Vertex, typename Index> struct indexed_mesh {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;

        using vertex_t = Vertex;
        using index_t  = Index;
        constexpr static inline bool indexed = true;


        void clear(void) {
            vertices.clear();
            indices.clear();
        }
    };


    using color_mesh    = indexed_mesh<vt::color_vertex_3d,    u32>;
    using textured_mesh = indexed_mesh<vt::texture_vertex_3d,  u32>;
    using material_mesh = indexed_mesh<vt::material_vertex_3d, u32>;

    using unindexed_color_mesh    = unindexed_mesh<vt::color_vertex_3d>;
    using unindexed_textured_mesh = unindexed_mesh<vt::texture_vertex_3d>;
    using unindexed_material_mesh = unindexed_mesh<vt::material_vertex_3d>;
}