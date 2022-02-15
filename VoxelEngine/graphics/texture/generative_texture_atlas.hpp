#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>
#include <VoxelEngine/graphics/texture/aligned_texture_atlas.hpp>


namespace ve::gfx {
    template <typename Atlas> class generative_texture_atlas : public texture_atlas<generative_texture_atlas<Atlas>> {
    public:
        explicit generative_texture_atlas(std::string name = "textures", std::function<unique<Atlas>(void)> constructor = [] { return make_unique<Atlas>(); }) :
            name(std::move(name)), constructor(std::move(constructor))
        {
            atlases.emplace_back(this->constructor());
        }


        texture_list get_uniform_textures(void) const override {
            return atlases | views::indirect | views::transform(ve_get_field(get_texture())) | ranges::to<std::vector>();
        }


        std::string get_uniform_name(void) const override {
            return name;
        }


        vec2ui size(void) const {
            return atlases[0]->size();
        }


        vec2ui quantization(void) const {
            return atlases[0]->quantization();
        }


        subtexture prepare_storage(const vec2ui& size) {
            return invoke_until_success([&] (auto& atlas, u8 i) {
                auto st = atlas->prepare_storage(size);
                st.binding = i;

                return st;
            });
        }


        void store_at(const image_rgba8& img, subtexture& where) {
            atlases.at(where.binding)->store_at(img, where);
        }


        std::vector<subtexture> prepare_storage_for_all(const std::vector<vec2ui>& sizes) {
            return invoke_until_success([&] (auto& atlas, u8 i) {
                auto sts = atlas->prepare_storage_for_all(sizes);
                for (auto& st : sts) st.binding = i;

                return sts;
            });
        }


        VE_GET_CREF(name);
    private:
        std::string name;
        std::vector<unique<Atlas>> atlases;
        std::function<unique<Atlas>(void)> constructor;


        // Invokes the provided function as fn(atlas, index) for every atlas until it succeeds without throwing.
        auto invoke_until_success(auto fn) {
            for (auto [i, atlas] : atlases | views::enumerate) {
                try {
                    return std::invoke(fn, atlas, (u8) i);
                } catch (const atlas_full_error &e) {}
            }

            if (atlases.size() >= max_value<u8>) {
                throw atlas_full_error { "Generative atlas cannot generate more atlases: u8 max value exceeded." };
            }

            auto& atlas = atlases.emplace_back(constructor());
            return std::invoke(fn, atlas, (u8) (atlases.size() - 1));
        }
    };


    using aligned_generative_texture_atlas = generative_texture_atlas<aligned_texture_atlas>;
}