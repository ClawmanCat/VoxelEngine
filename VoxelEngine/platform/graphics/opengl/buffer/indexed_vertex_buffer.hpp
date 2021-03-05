#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer_utils.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/attribute.hpp>

#include <GL/glew.h>

#include <type_traits>


namespace ve::graphics {
    // Vertex buffer with indexing. (i.e. each vertex may be drawn multiple times, controlled by the index buffer.)
    template <typename Vertex, typename Index = u32> class indexed_vertex_buffer : public buffer {
    public:
        indexed_vertex_buffer(GLenum primitive = GL_TRIANGLES, GLenum storage_mode = GL_DYNAMIC_DRAW) :
            vao(0), vbo(), ebo(), primitive(primitive)
        {
            glGenVertexArrays(1, &vao);
            VE_ASSERT(vao, "Failed to construct OpenGL VAO.");
            
            
            glGenBuffers(1, &vbo.buffer_id);
            VE_ASSERT(vbo, "Failed to construct OpenGL VBO.");
            
            vbo.buffer_type  = GL_ARRAY_BUFFER;
            vbo.storage_mode = storage_mode;
    
            
            glGenBuffers(1, &ebo.buffer_id);
            VE_ASSERT(ebo, "Failed to construct OpenGL EBO.");
            
            ebo.buffer_type  = GL_ELEMENT_ARRAY_BUFFER;
            ebo.storage_mode = storage_mode;
        }
    
    
        indexed_vertex_buffer(span<Vertex> auto&& vertices, span<Index> auto&& indices, GLenum primitive = GL_TRIANGLES, GLenum storage_mode = GL_DYNAMIC_DRAW) :
            indexed_vertex_buffer(primitive, storage_mode)
        {
            update_vertices(vertices);
            update_indices(indices);
        }
    
    
        indexed_vertex_buffer(const indexed_vertex_buffer&) = delete;
        indexed_vertex_buffer& operator=(const indexed_vertex_buffer&) = delete;
    
    
        indexed_vertex_buffer(indexed_vertex_buffer&& other) : buffer(std::move(other)), vao(0), vbo(), ebo() {
            std::swap(vao, other.vao);
            std::swap(vbo, other.vbo);
            std::swap(primitive, other.primitive);
        }
    
    
        indexed_vertex_buffer& operator=(indexed_vertex_buffer&& other) {
            buffer::operator=(std::move(other));
            
            std::swap(vao, other.vao);
            std::swap(vbo, other.vbo);
            std::swap(ebo, other.ebo);
            std::swap(primitive, other.primitive);
            
            return *this;
        }
        
        
        ~indexed_vertex_buffer(void) {
            if (vao) glDeleteVertexArrays(1, &vao);
            if (vbo) glDeleteBuffers(1, &vbo.buffer_id);
            if (ebo) glDeleteBuffers(1, &ebo.buffer_id);
        }
        
        
        virtual void draw(context& ctx) const override {
            buffer::draw(ctx);
            
            glBindVertexArray(vao);
            glBindBuffer(vbo.buffer_type, vbo.buffer_id);
            glBindBuffer(ebo.buffer_type, ebo.buffer_id);
            bind_vertex_layout<Vertex>(ctx);
            
            glDrawElements(primitive, ebo.size, detail::get_gl_type<Index>(), nullptr);
        }
        
        
        void reserve_vertices(std::size_t capacity) {
            detail::buffer_reserve(vao, capacity, vbo);
        }
        
        void reserve_indices(std::size_t capacity) {
            detail::buffer_reserve(vao, capacity, ebo);
        }
        
        
        void update_vertices(span<Vertex> auto&& elements, std::size_t where = 0) {
            detail::buffer_store(vao, elements, where, vbo);
        }
    
        void update_indices(span<Index> auto&& elements, std::size_t where = 0) {
            detail::buffer_store(vao, elements, where, ebo);
        }
    
    
        virtual GLuint get_id(void) const override { return vao; }
    private:
        GLuint vao;
        detail::buffer_storage<Vertex> vbo;
        detail::buffer_storage<Index> ebo;
        GLenum primitive;
    };
}