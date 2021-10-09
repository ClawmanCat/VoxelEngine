#pragma once

#include <VoxelEngine/core/core.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(texture/texture.hpp)


namespace ve::gfx {
    using texture_list = std::vector<shared<gfxapi::texture>>;


    // Samplers, unlike other uniforms, cannot be part of a UBO in GLSL.
    // Create a separate interface for them, and handle them in uniform_storage.
    struct uniform_sampler {
        virtual ~uniform_sampler(void) = default;
        virtual texture_list get_uniform_textures(void) const = 0;
        virtual std::string get_uniform_name(void) const = 0;
    };
}