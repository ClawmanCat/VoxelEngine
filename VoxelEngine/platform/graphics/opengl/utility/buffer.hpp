#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    template <typename T> class buffer {
    public:
        buffer(void) = default;
        explicit buffer(GLenum type) : gl_type(type) {}

        ~buffer(void) {
            if (id) glDeleteBuffers(1, &id);
        }

        ve_swap_move_only(buffer, id, size, capacity, gl_type);
        ve_field_comparable(buffer, id);


        void bind(void) const {
            glBindBuffer(gl_type, id);
        }


        void write(const T* data, std::size_t count, std::size_t where = 0) {
            VE_ASSERT(gl_type != GL_INVALID_ENUM, "Cannot write to uninitialized buffer.");
            VE_ASSERT(where <= size, "Write beyond end of buffer would leave uninitialized data.");

            if (count == 0) return;
            reserve(where + count);

            glBindBuffer(gl_type, id);
            glBufferSubData(gl_type, where * sizeof(T), count * sizeof(T), data);

            size = std::max(size, where + count);
        }


        void reserve(std::size_t count) {
            VE_ASSERT(gl_type != GL_INVALID_ENUM, "Cannot reserve space for uninitialized buffer.");
            if (count <= capacity) return;

            GLuint new_id;
            glGenBuffers(1, &new_id);
            glBindBuffer(gl_type, new_id);

            glBufferData(gl_type, count * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
            if (size) glCopyNamedBufferSubData(id, new_id, 0, 0, size * sizeof(T));

            if (id) glDeleteBuffers(1, &id);
            id = new_id;
            capacity = count;
        }


        void shrink(std::size_t size) {
            VE_ASSERT(size <= this->size, "Cannot shrink buffer to larger than its original size.");
            this->size = size;
        }


        VE_GET_VAL(id);
        VE_GET_VAL(size);
        VE_GET_VAL(capacity);
        VE_GET_VAL(gl_type);
    private:
        GLuint id = 0;
        std::size_t size = 0, capacity = 0;
        GLenum gl_type = GL_INVALID_ENUM;
    };
}