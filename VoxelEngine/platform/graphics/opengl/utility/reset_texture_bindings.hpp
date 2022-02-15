#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/missing_texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/get.hpp>


namespace ve::gfx::opengl {
    inline void reset_texture_bindings(GLint count = gl_get<GLint>(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)) {
        static auto tex = texture::create(texture_format_RGBA8, gfx::missing_texture::color_texture.size, 1);
        tex->write(gfx::missing_texture::color_texture);

        for (GLint i = 0; i < count; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            tex->bind();
        }
    }
}