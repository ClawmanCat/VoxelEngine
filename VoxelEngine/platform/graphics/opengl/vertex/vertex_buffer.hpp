#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
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


        virtual void draw(render_context& ctx) const = 0;


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
            vbo(GL_ARRAY_BUFFER)
        {}


        void draw(render_context& ctx) const override {
            if (vbo.get_size() == 0) return;

            auto uniform_raii = raii_bind_uniforms(ctx);

            glBindVertexArray(vao);
            vbo.bind();

            ctx.renderpass->get_shader()->get_vertex_layout().bind();
            glDrawArrays((GLenum) ctx.renderpass->get_settings().topology, 0, (GLsizei) vbo.get_size());
        }


        void store_vertices(std::span<const Vertex> vertices, std::size_t where = 0) {
            vbo.write(vertices.data(), vertices.size(), where);
        }

        void reserve_vertices(std::size_t count) {
            vbo.reserve(count);
        }

        void shrink_vertices(std::size_t count) {
            vbo.shrink(count);
        }

    protected:
        buffer<Vertex> vbo;

        auto raii_bind_uniforms(render_context& ctx) const {
            return raii_function {
                [&] {
                    uniform_storage::push(ctx.uniform_state);
                    ctx.uniform_state.bind_state(ctx.renderpass->get_shader()->get_id());
                },
                [&] { uniform_storage::pop(ctx.uniform_state); }
            };
        }
    };


    template <typename Vertex, typename Index> requires meta::pack<u8, u16, u32>::template contains<Index>
    class indexed_vertex_buffer : public unindexed_vertex_buffer<Vertex> {
    public:
        ve_shared_only(indexed_vertex_buffer) :
            unindexed_vertex_buffer<Vertex>(),
            ebo(GL_ELEMENT_ARRAY_BUFFER)
        {
            vertex_buffer::index_type = ctti::type_id<Index>();
        }


        void draw(render_context& ctx) const override {
            if (ebo.get_size() == 0) return;

            auto uniform_raii = raii_bind_uniforms(ctx);

            glBindVertexArray(vao);
            vbo.bind();
            ebo.bind();

            ctx.renderpass->get_shader()->get_vertex_layout().bind();
            glDrawElements((GLenum) ctx.renderpass->get_settings().topology, ebo.get_size(), get_index_type(), nullptr);
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


        constexpr static GLenum get_index_type(void) {
            if constexpr (std::is_same_v<Index, u8>) {
                return GL_UNSIGNED_BYTE;
            }

            else if constexpr (std::is_same_v<Index, u16>) {
                return GL_UNSIGNED_SHORT;
            }

            else if constexpr (std::is_same_v<Index, u32>) {
                return GL_UNSIGNED_INT;
            }

            else {
                static_assert(meta::always_false_v<Index>, "Unsupported EBO type.");
            }
        }
    };
}