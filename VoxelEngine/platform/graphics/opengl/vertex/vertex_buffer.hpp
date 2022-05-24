#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/graphics/vertex/mesh.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/ratio.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/buffer.hpp>

#include <gl/glew.h>
#include <ctti/type_id.hpp>


namespace ve::gfx::opengl {
    namespace detail {
        using vbo_overallocate_factor = std::ratio<3, 2>;


        inline auto raii_bind_uniforms(const uniform_storage& self, render_context& ctx) {
            return raii_function {
                [&] {
                    ctx.uniform_state.push_uniforms(&self);
                    ctx.uniform_state.bind_uniforms_for_shader(ctx.renderpass->get_shader().get());
                },
                [&] { ctx.uniform_state.pop_uniforms(); }
            };
        }
    }


    class vertex_buffer : public uniform_storage {
    public:
        ve_shared_only(vertex_buffer, ctti::type_index vertex_type, ctti::type_index index_type = meta::null_type_id)
            : uniform_storage(), vertex_type(std::move(vertex_type)), index_type(std::move(index_type))
        {}

        virtual ~vertex_buffer(void) = default;
        ve_immovable(vertex_buffer);


        virtual void draw(render_context& ctx) const = 0;


        VE_GET_CREF(vertex_type);
        VE_GET_CREF(index_type);
    protected:
        ctti::type_index vertex_type, index_type;
    };


    template <typename Vertex, typename Index = meta::null_type>
    class compound_vertex_buffer : public vertex_buffer {
    public:
        using buffer_handle = u32;


        ve_derived_shared_only(compound_vertex_buffer, vertex_buffer) :
            vertex_buffer(ctti::type_id<Vertex>(), ctti::type_id<Index>())
        {}


        void draw(render_context& ctx) const override {
            VE_PROFILE_FN();

            ctx.uniform_state.push_uniforms(this);
            for (const auto& [id, buffer] : buffers) buffer->draw(ctx);
            ctx.uniform_state.pop_uniforms();
        }


        buffer_handle insert(shared<vertex_buffer> buffer) {
            VE_DEBUG_ASSERT(
                (buffer->get_vertex_type() == ctti::type_id<Vertex>() && buffer->get_index_type() == ctti::type_id<Index>()),
                "Sub-buffers of compound vertex buffer must have matching vertex and index types."
            );

            buffers.emplace(next_handle, std::move(buffer));
            return next_handle++;
        }


        void erase(buffer_handle handle) {
            buffers.erase(handle);
        }


        shared<vertex_buffer> get(buffer_handle handle) {
            return buffers.at(handle);
        }
    private:
        hash_map<buffer_handle, shared<vertex_buffer>> buffers;
        buffer_handle next_handle = 0;
    };


    template <typename Vertex> requires has_vertex_layout<Vertex>
    class unindexed_vertex_buffer : public vertex_buffer {
    public:
        ve_derived_shared_only(unindexed_vertex_buffer, vertex_buffer) :
            vertex_buffer(ctti::type_id<Vertex>()),
            vbo(GL_ARRAY_BUFFER)
        {}

        ~unindexed_vertex_buffer(void) {
            if (vao) glDeleteVertexArrays(1, &vao);
        }


        void draw(render_context& ctx) const override {
            VE_PROFILE_FN();


            VE_DEBUG_ASSERT(
                ctx.renderpass->get_shader()->get_vertex_type() == get_vertex_type(),
                "Attempt to render buffer using shader made to render a different type of vertex."
            );


            if (vbo.get_size() == 0) return;

            auto uniform_raii = detail::raii_bind_uniforms(*this, ctx);

            create_vao();
            glBindVertexArray(vao);

            vbo.bind();

            // TODO: Cache vertex layout using VAO.
            ctx.renderpass->get_shader()->get_vertex_layout_reflection().bind();
            glDrawArrays((GLenum) ctx.renderpass->get_settings().topology, 0, (GLsizei) vbo.get_size());
        }


        void store_mesh(const mesh_types::unindexed_mesh<Vertex>& mesh) {
            store_vertices(mesh.vertices);
            shrink_vertices(mesh.vertices.size());
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

        VE_GET_VAL(vao);
    protected:
        mutable GLuint vao = 0;
        buffer<Vertex, detail::vbo_overallocate_factor> vbo;

        void create_vao(void) const {
            if (!vao) [[unlikely]] {
                glCreateVertexArrays(1, &vao);
                VE_ASSERT(vao, "Failed to create OpenGL vertex array object.");
            }
        }
    };


    template <typename Vertex, typename Index> requires meta::pack<u8, u16, u32>::template contains<Index>
    class indexed_vertex_buffer : public unindexed_vertex_buffer<Vertex> {
    public:
        ve_derived_shared_only(indexed_vertex_buffer, unindexed_vertex_buffer) :
            unindexed_vertex_buffer<Vertex>(),
            ebo(GL_ELEMENT_ARRAY_BUFFER)
        {
            vertex_buffer::index_type = ctti::type_id<Index>();
        }

        ~indexed_vertex_buffer(void) {
            if (vao) glDeleteVertexArrays(1, &vao);
        }


        void draw(render_context& ctx) const override {
            VE_PROFILE_FN();


            VE_DEBUG_ASSERT(
                ctx.renderpass->get_shader()->get_vertex_type() == get_vertex_type(),
                "Attempt to render buffer using shader made to render a different type of vertex."
            );


            if (ebo.get_size() == 0) return;

            auto uniform_raii = detail::raii_bind_uniforms(*this, ctx);

            create_vao();
            glBindVertexArray(get_vao());

            vbo.bind();
            ebo.bind();

            ctx.renderpass->get_shader()->get_vertex_layout_reflection().bind();
            glDrawElements((GLenum) ctx.renderpass->get_settings().topology, ebo.get_size(), get_index_type(), nullptr);
        }


        // Don't allow construction from a vertex-only mesh like base class.
        void store_mesh(const mesh_types::unindexed_mesh<Vertex>& mesh) = delete;


        void store_mesh(const mesh_types::indexed_mesh<Vertex, Index>& mesh) {
            store_vertices(mesh.vertices);
            shrink_vertices(mesh.vertices.size());

            store_indices(mesh.indices);
            shrink_indices(mesh.indices.size());
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
        buffer<Index, detail::vbo_overallocate_factor> ebo;


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