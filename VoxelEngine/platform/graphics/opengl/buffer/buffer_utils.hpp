#pragma once

#include <VoxelEngine/core/core.hpp>

#include <GL/glew.h>


namespace ve::graphics::detail {
    template <typename Stored> struct buffer_storage {
        using stored_t = Stored;
        
        GLuint buffer_id = 0;
        std::size_t size = 0, capacity = 0;
        GLenum storage_mode = GL_INVALID_ENUM;
        GLenum buffer_type = GL_INVALID_ENUM;
        
        
        // Allows checking if buffer is valid with if (buffer) { ... }
        constexpr operator bool(void) const { return (bool) buffer_id; }
    };
    
    
    // Updates a buffer such that it can hold at least new_capacity elements.
    template <typename Stored>
    inline void buffer_reserve(GLuint vao, std::size_t new_capacity, buffer_storage<Stored>& storage) {
        if (new_capacity <= storage.capacity) return;
    
        glBindVertexArray(vao);
    
        GLuint new_buffer;
        glGenBuffers(1, &new_buffer);
        VE_ASSERT(new_buffer, "Failed to create OpenGL buffer.");
    
        glBindBuffer(storage.buffer_type, new_buffer);
        glBufferData(storage.buffer_type, new_capacity * sizeof(Stored), nullptr, storage.storage_mode);
        
        if (storage.size > 0) {
            glCopyBufferSubData(storage.buffer_id, new_buffer, 0, 0, storage.size * sizeof(Stored));
        }
        
        glDeleteBuffers(1, &storage.buffer_id);
        
        storage.buffer_id = new_buffer;
        storage.capacity  = new_capacity;
    }
    
    
    // Stores the provided objects at the provided position in the buffer. Expands storage if necessary.
    template <typename Stored>
    inline void buffer_store(GLuint vao, std::span<const Stored> data, std::size_t where, buffer_storage<Stored>& storage) {
        VE_ASSERT(where <= storage.size, "Cannot insert into buffer at position which would cause uninitialized elements to be drawn.");
        
        if (data.size() == 0) return;
        buffer_reserve(vao, where + data.size(), storage);
        
        glBindVertexArray(vao);
        glBindBuffer(storage.buffer_type, storage.buffer_id);
        glBufferSubData(storage.buffer_type, where * sizeof(Stored), data.size_bytes(), &data.front());
        
        storage.size = where + data.size();
    }
}