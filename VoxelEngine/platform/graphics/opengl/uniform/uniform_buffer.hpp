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

        VE_DEBUG_ONLY(ve_rt_swap_move_only(uniform_buffer, reflection, ubo, current_value, written));
        VE_RELEASE_ONLY(ve_rt_swap_move_only(uniform_buffer, reflection, ubo, current_value));


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

            if (data.size() == current_value.size() && memcmp(current_value.data(), data.data(), data.size()) == 0) {
                // Stored value is equal to current value, do nothing.
                return;
            }

            current_value.resize(data.size());
            memcpy(current_value.data(), data.data(), data.size());

            ubo.write(data.data(), data.size());
            VE_DEBUG_ONLY(written = true);
        }
    private:
        const reflect::attribute* reflection;

        // UBOs are typically very small, so its worth caching the value CPU side so we can skip redundant writes.
        buffer<u8> ubo;
        std::vector<u8> current_value;

        VE_DEBUG_ONLY(bool written = false);
    };
}