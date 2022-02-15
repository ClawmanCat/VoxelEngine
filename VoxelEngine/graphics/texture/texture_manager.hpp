#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/thread/thread_pool.hpp>
#include <VoxelEngine/graphics/texture/missing_texture.hpp>
#include <VoxelEngine/graphics/texture/texture_source.hpp>
#include <VoxelEngine/graphics/texture/generative_texture_atlas.hpp>

#include <shared_mutex>


namespace ve::gfx {
    // Manages the loading and caching of textures from different sources.
    // This class is thread-safe, even if the underlying atlas or graphics API is not.
    template <typename Atlas = aligned_generative_texture_atlas>
    class texture_manager {
    public:
        using named_image_list = std::vector<std::pair<std::string_view, const image_rgba8*>>;


        template <typename... Args> explicit texture_manager(Args&&... args) : atlas(make_shared<Atlas>(fwd(args)...)) {}


        // Loads the texture from the given source, if it is not cached already, otherwise returns the cached value.
        subtexture get_or_load(texture_source& src) {
            std::unique_lock lock { mtx };

            const std::string name = src.name();
            if (auto tex = get_if_exists(name); tex) return *tex;

            const image_rgba8* img = src.require();
            raii_function release_img { no_op, [&] { src.relinquish(img); } };

            return load_to_common_texture(views::single(std::pair { name, img }), lock)[0];
        }


        // Same as above, but guarantees that all subtextures will be part of the same texture.
        // If the combination of images is too large to fit on a single texture, this method throws.
        std::vector<subtexture> get_or_load_to_common_texture(const std::vector<texture_source*>& sources) {
            std::unique_lock lock { mtx };

            auto names = sources | views::indirect | views::transform(&texture_source::name);
            if (auto textures = get_if_exists_common_texture(names); !textures.empty()) return textures;

            std::vector<const image_rgba8*> acquired;
            raii_function release_images { no_op, [&] {
                for (const auto& [i, ptr] : acquired | views::enumerate) sources[i]->relinquish(ptr);
            } };

            acquired = sources | views::indirect | views::transform(&texture_source::require) | ranges::to<std::vector>;
            return load_to_common_texture(views::zip(names, acquired), lock);
        }


        subtexture get_or_load(const fs::path& path, const image_rgba8& fallback = missing_texture::color_texture) {
            file_image_source src { &path, &fallback };
            return get_or_load(src);
        }

        subtexture get_or_load(std::string name, const image_rgba8& img) {
            direct_image_source src { std::move(name), &img };
            return get_or_load(src);
        }

        // Invokes the given function to load the texture only if it is not loaded already.
        template <typename Pred> requires std::is_invocable_r_v<image_rgba8, Pred>
        subtexture get_or_load(std::string name, Pred pred) {
            generative_image_source<Pred> src { std::move(name), std::move(pred) };
            return get_or_load(src);
        }


        void remove_texture(std::string_view name) {
            std::unique_lock lock { mtx };

            auto it = subtextures.find(name);
            if (it == subtextures.end()) return;

            atlas.remove(it->second);
            subtextures.erase(it);
        }


        bool is_loaded(std::string_view name) const {
            std::shared_lock lock { mtx };
            return subtextures.contains(name);
        }


        VE_GET_CREF(atlas);
    private:
        shared<Atlas> atlas;
        hash_map<std::string, subtexture> subtextures;


        // Note on thread safety:
        // Due to the requirement of this class being threadsafe, the addition of subtextures to the atlas must occur as a single transaction,
        // or it would be possible for the same texture to be inserted multiple times.
        // The issue with this is that we can't do a locked wait for the main thread (required to do graphics API calls), since the main
        // thread may itself also access the texture manager and thus try to acquire the mutex.
        // The solution is to reserve storage in the atlas, but not fill it yet until the main thread gets around to it.
        // This means that subtextures returned by this class' method might not yet be valid, which isn't really an issue, since it takes the
        // main thread to do anything graphics related anyway.
        // Worst case scenario a broken texture is observed for a single frame until the main thread patches it.
        // TODO: This is not ideal and a better implementation should be found later.
        mutable std::shared_mutex mtx;


        // Note: calling method is responsible for acquiring lock.
        subtexture* get_if_exists(std::string_view name) {
            if (auto it = subtextures.find(name); it != subtextures.end()) {
                return &(it->second);
            }

            return nullptr;
        }


        // Returns one of the following:
        // - If none of the textures in the range are loaded, returns an empty vector.
        // - If all of the textures in the range are loaded, and have the same atlas texture, returns those textures.
        // - If only some of the textures are loaded, triggers an assert.
        // - If two or more of the textures have different atlas textures, triggers an assert.
        // Note: calling method is responsible for acquiring lock.
        // TODO: These asserts could probably be resolved by moving textures around when a new requirement is imposed.
        std::vector<subtexture> get_if_exists_common_texture(const auto& names) {
            std::vector<subtexture> result;
            u8 texture_id;
            enum { UNDECIDED, UNLOADED, LOADED } state = UNDECIDED;


            for (const auto& name : names) {
                if (auto tex = get_if_exists(name); tex) {
                    if (state == UNDECIDED) {
                        texture_id = tex->binding;
                        state = LOADED;
                    }

                    else if (state == LOADED) {
                        VE_ASSERT(
                            texture_id == tex->binding,
                            "Attempt to load multiple subtextures to a single atlas texture failed "
                            "because some subtextures were already loaded to different atlas textures."
                        );
                    }

                    else {
                        VE_ASSERT(
                            false,
                            "Attempt to load multiple subtextures to a single atlas texture failed: "
                            "this method does not support mixing loaded and unloaded textures."
                        );
                    }

                    result.push_back(*tex);
                }

                else {
                    VE_ASSERT(
                        state != LOADED,
                        "Attempt to load multiple subtextures to a single atlas texture failed: "
                        "this method does not support mixing loaded and unloaded textures."
                    );

                    state = UNLOADED;
                }
            }


            return result;
        }


        // Loads the given images to a common atlas texture. Images should be a range of pairs [name : stringlike, img : image_rgba8*]
        // Does not perform checks for whether or not images already exist.
        // Note: calling method is responsible for acquiring lock. Method will release it while waiting to prevent deadlock.
        std::vector<subtexture> load_to_common_texture(const auto& images, auto& lock) {
            std::vector<subtexture> result = atlas->prepare_storage_for_all(
                images | views::values | views::indirect | views::transform(&image_rgba8::size) | ranges::to<std::vector>
            );

            for (const auto& [name, tex] : views::zip(images | views::keys, result)) {
                subtextures.emplace(std::string { name }, tex);
            }

            lock.unlock();


            // Actual graphics API actions are performed on the main thread since some APIs (notably OpenGL)
            // don't support calls from different threads, even if they are not ran in parallel.
            thread_pool::instance().invoke_on_main_or_run([&] {
                std::unique_lock lock { mtx };
                atlas->store_all_at(images | views::values | ranges::to<std::vector>, result);
            });


            return result;
        }
    };
}