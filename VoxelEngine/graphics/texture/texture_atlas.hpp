#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(texture/texture.hpp)


namespace ve::gfx {
    struct atlas_full_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };


    // Represents a region of a texture in a texture atlas.
    struct subtexture {
        shared<gfxapi::texture> parent;

        vec2f uv;
        vec2f wh;

        u8 binding;
    };


    template <typename Derived> struct texture_atlas : public uniform_sampler {
        vec2ui size(void) const {
            VE_CRTP_CHECK(Derived, size);
            return static_cast<const Derived*>(this)->size();
        }


        // Represents the alignment of subtextures within this atlas.
        // Smaller textures may be stored, but will leave unused padding around them.
        vec2ui quantization(void) const {
            VE_CRTP_CHECK(Derived, quantization);
            return static_cast<const Derived*>(this)->quantization();
        }


        // Attempts to store the given image in the atlas. Throws atlas_full_error if the atlas is full.
        subtexture store(const image_rgba8& img) {
            VE_CRTP_CHECK(Derived, store);
            return static_cast<Derived*>(this)->store(img);
        }


        void remove(const subtexture& st) {
            VE_CRTP_CHECK(Derived, remove);
            return static_cast<Derived*>(this)->remove(st);
        }


        // Either stores all images, or none of them if not all of them fit.
        // Throws atlas_full_error if the images were not inserted.
        std::vector<subtexture> store_all(const std::vector<const image_rgba8*>& images) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, store_all)) {
                return static_cast<Derived*>(this)->store_all(images);
            } else {
                std::vector<subtexture> result;

                try {
                    for (const auto* img : images) result.push_back(store(*img));
                } catch (const atlas_full_error& e) {
                    for (const auto& st : result) remove(st);
                    throw;
                }

                return result;
            }
        }
    };
}