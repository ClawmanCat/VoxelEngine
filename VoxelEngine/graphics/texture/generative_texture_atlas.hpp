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


        subtexture store(const image_rgba8& img) {
            // Attempt inserting into an existing atlas.
            for (auto& [i, atlas] : atlases | views::enumerate) {
                try {
                    return set_bindings(atlas->store(img), (u8) i);
                } catch (const atlas_full_error &e) {}
            }

            // Insert into a new atlas. If the insert still fails, the texture is just too big.
            atlases.push_back(constructor());
            return set_bindings(atlases.back()->store(img), u8(atlases.size() - 1));
        }


        void remove(const subtexture& st) {
            return atlases.at(st.binding)->remove(st);
        }


        std::vector<subtexture> store_all(const std::vector<const image_rgba8*>& images) {
            // Attempt inserting into an existing atlas.
            for (auto [i, atlas] : atlases | views::enumerate) {
                try {
                    return set_bindings(atlas->store_all(images), (u8) i);
                } catch (const atlas_full_error &e) {}
            }

            // Insert into a new atlas. If the insert still fails, the textures are just too big.
            atlases.push_back(constructor());
            return set_bindings(atlases.back()->store_all(images), u8(atlases.size() - 1));
        }


        VE_GET_CREF(name);
    private:
        std::string name;
        std::vector<unique<Atlas>> atlases;
        std::function<unique<Atlas>(void)> constructor;


        static std::vector<subtexture> set_bindings(std::vector<subtexture> src, u8 binding) {
            for (auto& st : src) st.binding = binding;
            return src;
        }

        static subtexture set_bindings(subtexture src, u8 binding) {
            src.binding = binding;
            return src;
        }
    };


    using aligned_generative_texture_atlas = generative_texture_atlas<aligned_texture_atlas>;
}