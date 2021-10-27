#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    template <typename T> requires std::is_scalar_v<T>
    inline T gl_get(GLenum type) {
        T result;

        if constexpr (std::is_same_v<T, bool>) {
            glGetBooleanv(type, &result);
        }

        else if constexpr (std::is_same_v<T, i32>) {
            glGetIntegerv(type, &result);
        }

        else if constexpr (std::is_same_v<T, float>) {
            glGetFloatv(type, &result);
        }

        else {
            static_assert(meta::always_false_v<T>, "No matching glGet function for provided type.");
        }

        return result;
    }
}