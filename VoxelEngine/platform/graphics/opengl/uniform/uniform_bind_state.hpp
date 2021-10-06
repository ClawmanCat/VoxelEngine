#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_buffer.hpp>


namespace ve::gfx::opengl {
    class uniform_storage;
    class shader;


    struct uniform_bind_state {
        struct combined_value {
            const void* value;
            std::vector<u8> value_std140;

            // Updating the UBO is just updating the GPUs view of the data, so allow this as a const action.
            mutable uniform_buffer ubo;
            mutable bool ubo_dirty = true;

            const reflect::object_type* type;


            // Pushes the current value of the uniform to the GPU. Call this before binding the UBO.
            void synchronize_ubo(void) const {
                VE_ASSERT(value, "Attempt to store uninitialized uniform in UBO.");

                if (std::exchange(ubo_dirty, false)) {
                    ubo.store_bytes(value_std140);
                }
            }
        };

        const shader* storage_owner = nullptr;
        std::stack<const uniform_storage*> uniform_stack;
        hash_map<std::string_view, combined_value> bound_uniforms;

        // When a UBO contains a single element, e.g. uniform Transform { mat4f transform; };
        // it is often desirable to set the uniform by the name of its member, rather than by creating a separate
        // struct to wrap it.
        // Keep track of what UBOs only have a single member, and what UBO those member names map to.
        hash_map<std::string_view, std::string_view> aliases;
    };
}