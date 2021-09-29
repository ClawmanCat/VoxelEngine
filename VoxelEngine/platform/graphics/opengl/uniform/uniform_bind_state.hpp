#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/spirtype.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform.hpp>


namespace ve::gfx::opengl {
    class uniform_storage;


    struct uniform_bind_state {
        struct combined_value {
            const void* value;
            const reflect::primitive_t* type;
        };

        std::stack<const uniform_storage*> uniform_stack;
        hash_map<std::string, combined_value> bound_uniforms;
    };
}