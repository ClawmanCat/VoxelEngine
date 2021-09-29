#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/buffer.hpp>

#include <gl/glew.h>
#include <ctti/type_id.hpp>


namespace ve::gfx::opengl {
    class vertex_buffer : public uniform_storage {
    public:
        ve_shared_only(vertex_buffer, ctti::type_index vertex_type, ctti::type_index index_type = meta::null_type_id)
            : uniform_storage(), vertex_type(std::move(vertex_type)), index_type(std::move(index_type))
        {
            glCreateVertexArrays(1, &vao);
            VE_ASSERT(vao, "Failed to create OpenGL vertex array object.");
        }

        virtual ~vertex_buffer(void) {
            glDeleteVertexArrays(1, &vao);
        }

        ve_immovable(vertex_buffer);


        virtual void draw(const render_context& ctx) = 0;


        VE_GET_VAL(vao);
        VE_GET_CREF(vertex_type);
        VE_GET_CREF(index_type);
    protected:
        GLuint vao;
        ctti::type_index vertex_type, index_type;
    };


    template <typename Vertex> requires has_vertex_layout<Vertex>
    class unindexed_vertex_buffer : public vertex_buffer {
    public:
        ve_shared_only(unindexed_vertex_buffer) :
            vertex_buffer(ctti::type_id<Vertex>()),
            vbo(vao, GL_ARRAY_BUFFER)
        {}


        void store_vertices(std::span<const Vertex> vertices, std::size_t where = 0) {
            vbo.write(vertices.data(), vertices.size(), where);
        }

        void reserve_vertices(std::size_t count) {
            vbo.reserve(count);
        }

        void shrink_vertices(std::size_t count) {
            vbo.shrink(count);
        }
    private:
        buffer<Vertex> vbo;
    };


    template <typename Vertex, typename Index> requires std::is_unsigned_v<Index>
    class indexed_vertex_buffer : public unindexed_vertex_buffer<Vertex> {
    public:
        ve_shared_only(indexed_vertex_buffer) :
            unindexed_vertex_buffer<Vertex>(),
            ebo(vao, GL_ELEMENT_ARRAY_BUFFER)
        {
            vertex_buffer::index_type = ctti::type_id<Index>();
        }


        void store_indices(std::span<const Index> indices, std::size_t where = 0) {
            ebo.write(indices.data(), indices.size(), where);
        }

        void reserve_indices(std::size_t count) {
            ebo.reserve(count);
        }

        void shrink_indices(std::size_t count) {
            ebo.shrink(count);
        }
    private:
        buffer<Index> ebo;
    };
}