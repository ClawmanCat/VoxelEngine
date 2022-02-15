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
            return VE_CRTP_CALL(Derived, size);
        }


        // Represents the alignment of subtextures within this atlas.
        // Smaller textures may be stored, but will leave unused padding around them.
        vec2ui quantization(void) const {
            return VE_CRTP_CALL(Derived, quantization);
        }


        // Removes the given subtexture from the atlas.
        // The subtexture may refer to an actual texture, added with store, or just a pre-allocated slot reserved with prepare_storage.
        void remove(const subtexture& st) {
            VE_CRTP_CALL(Derived, remove, st);
        }


        // Attempts to allocate space for the given image in the atlas, but does not yet perform a texture upload.
        // Throws atlas_full_error if the atlas is full.
        // This is useful in multithreaded environments, since graphics API calls must be made from the main thread.
        // Note: while this method may be called from worker threads, it is still responsible for preventing concurrent modification of the atlas object.
        subtexture prepare_storage(const vec2ui& size) {
            return VE_CRTP_CALL(Derived, prepare_storage, size);
        }


        // Performs a texture upload for a texture previously allocated with prepare_storage.
        void store_at(const image_rgba8& img, subtexture& where) {
            VE_CRTP_CALL(Derived, store_at, img, where);
        }


        // Attempts to store the given image in the atlas. Throws atlas_full_error if the atlas is full.
        subtexture store(const image_rgba8& img) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, store)) {
                return VE_CRTP_CALL(Derived, store, img);
            } else {
                auto where = prepare_storage(img.size);
                store_at(img, where);
                return where;
            }
        }


        // Attempts to allocate space for the given images in the atlas, but does not yet perform a texture upload.
        // This is useful in multithreaded environments, since graphics API calls must be made from the main thread.
        // If there is no room to store all provided images on the same texture, an atlas_full_error is thrown.
        // Note: while this method may be called from worker threads, it is still responsible for preventing concurrent modification of the atlas object.
        std::vector<subtexture> prepare_storage_for_all(const std::vector<vec2ui>& sizes) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, prepare_storage_for_all)) {
                return VE_CRTP_CALL(Derived, prepare_storage_for_all, sizes);
            } else {
                std::vector<subtexture> result;

                try {
                    for (const auto& size : sizes) result.push_back(prepare_storage(size));
                } catch (const atlas_full_error& e) {
                    for (const auto& st : result) remove(st);
                    throw;
                }

                return result;
            }
        }


        // Either stores all images, or none of them if not all of them fit.
        // Throws atlas_full_error if the images were not inserted.
        std::vector<subtexture> store_all(const std::vector<const image_rgba8*>& images) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, store_all)) {
                return VE_CRTP_CALL(Derived, store_all, images);
            } else {
                auto sizes = images | views::indirect | views::transform(&image_rgba8::size) | ranges::to<std::vector>;
                auto where = prepare_storage_for_all(sizes);

                for (const auto& [img, pos] : views::zip(images, where)) {
                    store_at(*img, pos);
                }

                return where;
            }
        }


        void store_all_at(const std::vector<const image_rgba8*>& images, std::vector<subtexture>& where) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, store_all_at)) {
                return VE_CRTP_CALL(Derived, store_all_at, images, where);
            } else {
                for (auto [img, where] : views::zip(images, where)) store_at(*img, where);
            }
        }
    };
}