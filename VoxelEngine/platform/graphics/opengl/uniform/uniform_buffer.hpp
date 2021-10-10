#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/glsl_layout.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/buffer.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    class uniform_buffer {
    public:
        explicit uniform_buffer(const reflect::attribute* reflection) :
            reflection(reflection),
            ubo(GL_UNIFORM_BUFFER)
        {
            ubo.reserve(reflection->struct_size);
        }

        VE_DEBUG_ONLY(ve_swap_move_only(uniform_buffer, reflection, ubo, written));
        VE_RELEASE_ONLY(ve_swap_move_only(uniform_buffer, reflection, ubo));


        void bind(void) const {
            VE_DEBUG_ASSERT(written, "Cannot bind UBO before writing data to it.");
            glBindBufferBase(GL_UNIFORM_BUFFER, reflection->binding, ubo.get_id());
        }


        template <typename T> void store_object(const T& object) {
            // Either the types must match, or the UBO must have one element and the type of that element must match.
            // This allows us to e.g. set an uniform like "uniform MyUniform { mat4 mat; }" with just a matrix rather than a struct containing one.
            VE_DEBUG_ASSERT(
                reflect::object_type { meta::type_wrapper<T>{} } == reflection->type ||
                (
                    reflection->members.size() == 1 &&
                    reflect::object_type { meta::type_wrapper<T>{} } == reflection->members[0].type
                ),
                "Cannot convert object of type ", ctti::nameof<T>(), " to UBO ", reflection->name, ": ",
                "The objects have different types and the UBO does not consist of a single element matching the type either."
            );

            store_bytes(to_std140(object));
        }


        void store_bytes(std::span<const u8> data) {
            VE_ASSERT(data.size() == reflection->struct_size, "Incorrect object type for UBO: size mismatch.");

            ubo.write(data.data(), data.size());
            VE_DEBUG_ONLY(written = true);
        }
    private:
        const reflect::attribute* reflection;
        buffer<u8> ubo;
        VE_DEBUG_ONLY(bool written = false);
    };
}