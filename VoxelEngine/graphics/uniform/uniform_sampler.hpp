#pragma once

#include <VoxelEngine/core/core.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(texture/texture.hpp)


namespace ve::gfx {
    using texture_list = std::vector<shared<gfxapi::texture_base>>;


    // Samplers, unlike other uniforms, cannot be part of a UBO in GLSL.
    // Create a separate interface for them, and handle them in uniform_storage.
    struct uniform_sampler {
        virtual ~uniform_sampler(void) = default;
        virtual texture_list get_uniform_textures(void) const = 0;
        virtual std::string get_uniform_name(void) const = 0;
    };


    // Simple wrapper around a texture to make it a uniform sampler.
    struct named_texture : public uniform_sampler {
        shared<gfxapi::texture_base> texture;
        std::string name;


        named_texture(shared<gfxapi::texture_base> texture, std::string name) :
            texture(std::move(texture)),
            name(std::move(name))
        {}


        texture_list get_uniform_textures(void) const override {
            return { texture };
        }


        std::string get_uniform_name(void) const override {
            return name;
        }
    };


    // Simple wrapper around a set of textures to make them into a uniform sampler.
    struct named_texture_array : public uniform_sampler {
        texture_list textures;
        std::string name;


        named_texture_array(texture_list textures, std::string name) :
            textures(std::move(textures)),
            name(std::move(name))
        {}


        texture_list get_uniform_textures(void) const override {
            return textures;
        }


        std::string get_uniform_name(void) const override {
            return name;
        }
    };
}