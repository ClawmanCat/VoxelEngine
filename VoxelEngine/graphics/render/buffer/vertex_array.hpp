#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/threading/threadsafe_counter.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>
#include <VoxelEngine/graphics/render/mesh/vertex_attribute.hpp>
#include <VoxelEngine/graphics/render/uniform_storage.hpp>

#include <GL/glew.h>
#include <magic_enum.hpp>

#include <type_traits>


namespace ve {
    struct vertex_array_base : public uniform_storage {
        vertex_array_base(void) : id(threadsafe_counter<vertex_array_base>::next()) {}
        
        virtual ~vertex_array_base(void) = default;
        virtual void draw(GLuint program, u32& uniform_state) = 0;
        
        [[nodiscard]] u64 get_id(void) const noexcept { return id; }
    private:
        u64 id = 0;
    };
    
    
    template <
        typename Vertex,
        bool Indexed = true,
        typename Index = u32
    > class vertex_array : public vertex_array_base {
    public:
        enum buffer_type { VERTEX_BUFFER = GL_ARRAY_BUFFER, INDEX_BUFFER = GL_ELEMENT_ARRAY_BUFFER };
    
        template <buffer_type type>
        constexpr static bool has_buffer = (type == VERTEX_BUFFER) || Indexed;
        
        template <buffer_type type>
        using buffer_elem_t = std::conditional_t<(type == VERTEX_BUFFER), Vertex, Index>;
        
        
        vertex_array(GLenum primitive = GL_TRIANGLES, GLenum draw_mode = GL_DYNAMIC_DRAW)
            : primitive(primitive), draw_mode(draw_mode), vao(0), vbo(0), verts(0), vert_alloc(0)
        {
            if constexpr (Indexed) {
                ebo = 0;
                indices = index_alloc = 0;
            }
            
            
            glGenVertexArrays(1, &vao);
            VE_ASSERT(vao);
            glBindVertexArray(vao);
            
            glGenBuffers(1, &vbo);
            VE_ASSERT(vbo);
            
            if constexpr (Indexed) {
                glGenBuffers(1, &ebo);
                VE_ASSERT(ebo);
            }
        }
    
    
        template <span<const Vertex> VertexSpan> requires (!Indexed)
        vertex_array(
            VertexSpan vertices,
            GLenum primitive = GL_TRIANGLES,
            GLenum draw_mode = GL_DYNAMIC_DRAW
        ) : vertex_array(primitive, draw_mode) {
            update<VERTEX_BUFFER>(vertices);
        }
    
    
        template <span<const Vertex> VertexSpan, span<const Index> IndexSpan> requires Indexed
        vertex_array(
            VertexSpan vertices,
            IndexSpan indices,
            GLenum primitive = GL_TRIANGLES,
            GLenum draw_mode = GL_DYNAMIC_DRAW
        ) : vertex_array(primitive, draw_mode) {
            update<VERTEX_BUFFER>(vertices);
            update<INDEX_BUFFER>(indices);
        }
        
        
        ~vertex_array(void) {
            if (vao) glDeleteVertexArrays(1, &vao);
            if (vbo) glDeleteBuffers(1, &vbo);
            
            if constexpr (Indexed) {
                if (ebo) glDeleteBuffers(1, &ebo);
            }
        }
        
        
        vertex_array(const vertex_array&) = delete;
        vertex_array& operator=(const vertex_array&) = delete;
    
        vertex_array(vertex_array&& o) {
            *this = std::move(o);
        }
        
        vertex_array& operator=(vertex_array&& o) {
            primitive = o.primitive;
            draw_mode = o.draw_mode;
            
            verts = o.verts;
            vert_alloc = o.vert_alloc;
            
            if constexpr (Indexed) {
                indices = o.indices;
                index_alloc = o.index_alloc;
            }
            
            vao = std::exchange(o.vao, 0);
            vbo = std::exchange(o.vbo, 0);
            if constexpr (Indexed) ebo = std::exchange(o.ebo, 0);
            
            return *this;
        }
        
    
        // Draws the buffer using the currently bound program.
        virtual void draw(GLuint program, u32& uniform_state) override {
            bind_uniforms(program, uniform_state);
            
            glBindVertexArray(vao);
            
            glBindBuffer(VERTEX_BUFFER, vbo);
            if constexpr (Indexed) glBindBuffer(INDEX_BUFFER, ebo);
            
            for (const vertex_attribute& attrib : Vertex::get_attributes()) {
                GLint index = glGetAttribLocation(program, attrib.name);
                if (index == -1) continue;
                
                glEnableVertexAttribArray(index);
                
                if (attrib.type == GL_FLOAT) {
                    glVertexAttribPointer(index, attrib.count, attrib.type, GL_FALSE, sizeof(Vertex), (void*) attrib.offset);
                } else if (attrib.type == GL_DOUBLE) {
                    glVertexAttribLPointer(index, attrib.count, attrib.type, sizeof(Vertex), (void*) attrib.offset);
                } else {
                    glVertexAttribIPointer(index, attrib.count, attrib.type, sizeof(Vertex), (void*) attrib.offset);
                }
            }
            
            if constexpr (Indexed) {
                glDrawElements(
                    primitive,
                    indices,
                    detail::get_gl_type<Index>(),
                    (void*) 0
                );
            } else glDrawArrays(primitive, 0, verts);
        }
        
        
        // Updates elements in the vertex or index buffer.
        // Storage is extended if necessary, but where must be less than the current size
        // to prevent uninitialized vertices being rendered.
        template <buffer_type type, span<const buffer_elem_t<type>> Span> requires has_buffer<type>
        void update(Span data, std::size_t where = 0) {
            VE_ASSERT(where <= size<type>() && data.size() > 0);
            
            
            reserve<type>(where + data.size());
            
            glBindVertexArray(vao);
            glBindBuffer(type, get_buffer<type>());
            glBufferSubData(type, where * sizeof(buffer_elem_t<type>), data.size_bytes(), &data.front());
            
            meta::return_if<(type == VERTEX_BUFFER)>(verts, indices) = where + data.size();
        }
        
        
        template <buffer_type type> requires has_buffer<type>
        [[nodiscard]] std::size_t size(void) const noexcept {
            return meta::return_if<(type == VERTEX_BUFFER)>(verts, indices);
        }
    
    
        template <buffer_type type> requires has_buffer<type>
        [[nodiscard]] std::size_t allocated_size(void) const noexcept {
            return meta::return_if<(type == VERTEX_BUFFER)>(vert_alloc, index_alloc);
        }
        
    private:
        template <typename T> using indexed_only = std::conditional_t<Indexed, T, meta::null_type>;
        
        GLenum primitive;
        GLenum draw_mode;
        
        GLuint vao, vbo;
        [[no_unique_address]] indexed_only<GLuint> ebo;
        
        std::size_t verts, vert_alloc;
        [[no_unique_address]] indexed_only<std::size_t> indices, index_alloc;
        
        
        template <buffer_type type> requires has_buffer<type>
        GLuint& get_buffer(void) {
            return meta::return_if<(type == VERTEX_BUFFER)>(vbo, ebo);
        }
        
        
        template <buffer_type type> requires has_buffer<type>
        void reserve(std::size_t new_size) {
            auto& size  = meta::return_if<(type == VERTEX_BUFFER)>(verts, indices);
            auto& alloc = meta::return_if<(type == VERTEX_BUFFER)>(vert_alloc, index_alloc);
            
            if (alloc >= new_size) return;
    
            
            glBindVertexArray(vao);
    
            GLuint new_buffer = 0;
            glGenBuffers(1, &new_buffer);
            VE_ASSERT(new_buffer);
            
            glBindBuffer(type, new_buffer);
            glBufferData(type, new_size * sizeof(buffer_elem_t<type>), nullptr, draw_mode);
            if (size > 0) glCopyBufferSubData(get_buffer<type>(), new_buffer, 0, 0, size * sizeof(buffer_elem_t<type>));
            glDeleteBuffers(1, &get_buffer<type>());
            
            alloc = new_size;
            get_buffer<type>() = new_buffer;
        }
    };
}