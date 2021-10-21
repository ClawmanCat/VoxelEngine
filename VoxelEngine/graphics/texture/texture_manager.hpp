#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/graphics/texture/missing_texture.hpp>
#include <VoxelEngine/graphics/texture/texture_source.hpp>
#include <VoxelEngine/graphics/texture/generative_texture_atlas.hpp>


namespace ve::gfx {
    template <typename Atlas = aligned_generative_texture_atlas>
    class texture_manager {
    public:
        using named_image_list = std::vector<std::pair<std::string_view, const image_rgba8*>>;


        template <typename... Args> explicit texture_manager(Args&&... args) : atlas(make_shared<Atlas>(fwd(args)...)) {}


        // Loads the texture from the given source, if it is not cached already, otherwise returns the cached value.
        subtexture get_or_load(texture_source& src) {
            const std::string name = src.name();
            if (auto tex = get_if_exists(name); tex) return *tex;

            const image_rgba8* img = src.require();
            raii_function release_img { no_op, [&] { src.relinquish(img); } };

            return load_to_common_texture(views::single(std::pair { name, img }))[0];
        }


        // Same as above, but guarantees that all subtextures will be part of the same texture.
        // If the combination of images is too large to fit on a single texture, this method throws.
        std::vector<subtexture> get_or_load_to_common_texture(const std::vector<texture_source*>& sources) {
            auto names = sources | views::indirect | views::transform(&texture_source::name);
            if (auto textures = get_if_exists_common_texture(names); !textures.empty()) return textures;

            std::vector<const image_rgba8*> acquired;
            raii_function release_images { no_op, [&] {
                for (const auto& [i, ptr] : acquired | views::enumerate) sources[i]->relinquish(ptr);
            } };

            acquired = sources | views::indirect | views::transform(&texture_source::require) | ranges::to<std::vector>;
            return load_to_common_texture(views::zip(names, acquired));
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
            auto it = subtextures.find(name);
            if (it == subtextures.end()) return;

            atlas.remove(it->second);
            subtextures.erase(it);
        }


        bool is_loaded(std::string_view name) const {
            return subtextures.contains(name);
        }


        VE_GET_CREF(atlas);
    private:
        shared<Atlas> atlas;
        hash_map<std::string, subtexture> subtextures;


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


        // Loads the given images to a common atlas texture.
        // Does not perform checks for whether or not images already exist.
        std::vector<subtexture> load_to_common_texture(const auto& images) {
            auto result = atlas->store_all(images | views::values | ranges::to<std::vector>);

            for (const auto& [name, tex] : views::zip(images | views::keys, result)) {
                subtextures.emplace(std::string { name }, tex);
            }

            return result;
        }
    };
}