#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/spirtype.hpp>
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

            const reflect::primitive_t* type;


            // Pushes the current value of the uniform to the GPU. Call this before binding the UBO.
            void synchronize_ubo(void) const {
                VE_ASSERT(value, "Attempt to store uninitialized uniform in UBO.");

                if (std::exchange(ubo_dirty, false)) {
                    ubo.store_bytes(value_std140);
                }
            }
        };

        std::stack<const uniform_storage*> uniform_stack;
        hash_map<std::string_view, combined_value> bound_uniforms;
        const shader* storage_owner = nullptr;
    };
}