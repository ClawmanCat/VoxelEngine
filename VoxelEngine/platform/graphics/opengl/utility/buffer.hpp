#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/traits/ratio.hpp>
#include <VoxelEngine/utility/thread/assert_main_thread.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    template <typename T, meta::ratio Overallocate = std::ratio<1, 1>> requires std::ratio_greater_equal_v<Overallocate, std::ratio<1, 1>>
    class buffer {
    public:
        buffer(void) = default;
        explicit buffer(GLenum type) : gl_type(type) {}

        ~buffer(void) {
            if (id) glDeleteBuffers(1, &id);
        }

        ve_swap_move_only(buffer, id, size, capacity, gl_type);
        ve_field_comparable(buffer, id);


        void bind(void) const {
            assert_main_thread();
            glBindBuffer(gl_type, id);
        }


        void write(const T* data, std::size_t count, std::size_t where = 0) {
            VE_PROFILE_FN();

            assert_main_thread();
            VE_ASSERT(gl_type != GL_INVALID_ENUM, "Cannot write to uninitialized buffer.");
            VE_ASSERT(where <= size, "Write beyond end of buffer would leave uninitialized data.");

            if (count == 0) return;
            reserve(where + count);

            glBindBuffer(gl_type, id);
            glBufferSubData(gl_type, where * sizeof(T), count * sizeof(T), data);

            size = std::max(size, where + count);
        }


        void reserve(std::size_t count) {
            assert_main_thread();

            VE_ASSERT(gl_type != GL_INVALID_ENUM, "Cannot reserve space for uninitialized buffer.");
            if (count <= capacity) return;


            // A lot of buffers will be written to exactly once, so it would be wasteful to over-allocate.
            // Only over-allocate if this isn't the first time we're performing a write, i.e. capacity > 0.
            constexpr float f = float(Overallocate::num) / float(Overallocate::den);
            std::size_t new_capacity = (capacity > 0) ? std::size_t(count * f) : count;

            GLuint new_id;
            glGenBuffers(1, &new_id);
            glBindBuffer(gl_type, new_id);

            glBufferData(gl_type, new_capacity * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
            if (size) glCopyNamedBufferSubData(id, new_id, 0, 0, size * sizeof(T));

            if (id) glDeleteBuffers(1, &id);
            id = new_id;
            capacity = new_capacity;
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