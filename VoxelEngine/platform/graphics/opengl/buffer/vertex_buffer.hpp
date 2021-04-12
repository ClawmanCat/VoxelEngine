#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/buffer_utils.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/attribute.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/vertex.hpp>

#include <GL/glew.h>
#include <boost/pfr.hpp>

#include <type_traits>


namespace ve::graphics {
    // Vertex buffer without indexing. (i.e. each vertex is drawn once, in the order it is present in the buffer.)
    template <typename Vertex> class vertex_buffer : public buffer {
    public:
        explicit vertex_buffer(GLenum primitive = GL_TRIANGLES, GLenum storage_mode = GL_DYNAMIC_DRAW) :
            buffer(), vao(0), vbo(), primitive(primitive)
        {
            glGenVertexArrays(1, &vao);
            VE_ASSERT(vao, "Failed to construct OpenGL VAO.");
            
            glGenBuffers(1, &vbo.buffer_id);
            VE_ASSERT(vbo, "Failed to construct OpenGL VBO.");
            
            vbo.buffer_type  = GL_ARRAY_BUFFER;
            vbo.storage_mode = storage_mode;
        }
        
        
        explicit vertex_buffer(std::span<const Vertex> data, GLenum primitive = GL_TRIANGLES, GLenum storage_mode = GL_DYNAMIC_DRAW) :
            vertex_buffer(primitive, storage_mode)
        {
            update(data);
        }
        
        
        explicit vertex_buffer(const unindexed_mesh<Vertex>& mesh, GLenum primivite = GL_TRIANGLES, GLenum storage_mode = GL_DYNAMIC_DRAW) :
            vertex_buffer(primitive, storage_mode)
        {
            update(mesh.vertices);
        }
        
        
        vertex_buffer(const vertex_buffer&) = delete;
        vertex_buffer& operator=(const vertex_buffer&) = delete;
        
        
        vertex_buffer(vertex_buffer&& other) : buffer(std::move(other)), vao(0), vbo() {
            std::swap(vao, other.vao);
            std::swap(vbo, other.vbo);
            std::swap(primitive, other.primitive);
        }
        
        
        vertex_buffer& operator=(vertex_buffer&& other) {
            buffer::operator=(std::move(other));
            
            std::swap(vao, other.vao);
            std::swap(vbo, other.vbo);
            std::swap(primitive, other.primitive);
            
            return *this;
        }
        
        
        ~vertex_buffer(void) {
            if (vao) glDeleteVertexArrays(1, &vao);
            if (vbo.buffer_id) glDeleteBuffers(1, &vbo.buffer_id);
        }
        
        
        virtual void draw(context& ctx) const override {
            buffer::draw(ctx);
            
            glBindVertexArray(vao);
            glBindBuffer(vbo.buffer_type, vbo.buffer_id);
            bind_vertex_layout<Vertex>(ctx);
            
            glDrawArrays(primitive, 0, vbo.size);
        }
        
        
        void reserve(std::size_t capacity) {
            detail::buffer_reserve(vao, capacity, vbo);
        }
        
        
        void update(std::span<const Vertex> elements, std::size_t where = 0) {
            detail::buffer_store(vao, elements, where, vbo);
        }
    
    
        virtual GLuint get_id(void) const override { return vao; }
    private:
        GLuint vao;
        detail::buffer_storage<Vertex> vbo;
        GLenum primitive;
    };
}