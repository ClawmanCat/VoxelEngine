#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx::opengl::blend_functions {
    // Normal blending: merges semi-transparent pixels together according to their opacity.
    inline void normal(void) {
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    }


    // For each channel, keeps the smallest of the two values.
    inline void keep_smallest(void) {
        glBlendEquation(GL_MIN);
    }


    // For each channel, keep the largest of the two values.
    inline void keep_largest(void) {
        glBlendEquation(GL_MAX);
    }
}