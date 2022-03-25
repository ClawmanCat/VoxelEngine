#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/texture/texture_atlas.hpp>


namespace ve::gfx {
    class aligned_texture_atlas : public texture_atlas<aligned_texture_atlas> {
    public:
        constexpr static float epsilon = 1e-6;


        explicit aligned_texture_atlas(
            std::string name = "textures",
            const vec2ui& size = vec2ui { 8192 },
            u32 alignment = 32,
            const gfxapi::texture_format& fmt = gfxapi::texture_format_RGBA8
        ) :
            name(std::move(name)),
            atlas_size(size),
            atlas_alignment(alignment),
            texture(gfxapi::texture::create(size, fmt))
        {
            VE_ASSERT(atlas_size % atlas_alignment == vec2ui { 0 }, "Atlas size must be divisible by alignment.");
        }


        texture_list get_uniform_textures(void) const override {
            return { texture };
        }


        std::string get_uniform_name(void) const override {
            return name;
        }


        vec2ui size(void) const {
            return atlas_size;
        }

        vec2ui quantization(void) const {
            return vec2ui { atlas_alignment };
        }


        void remove(const subtexture& st) {
            mark_storage<false>(
                unaligned_to_blockpos(vec2ui { st.uv * vec2f { atlas_size } }),
                unaligned_to_blockpos(vec2ui { st.wh * vec2f { atlas_size } })
            );
        }


        subtexture prepare_storage(const vec2ui& size) {
            auto block_size = unaligned_to_blockpos(size);
            auto location   = find_storage(block_size);

            mark_storage<true>(location, block_size);

            auto st = subtexture {
                .parent  = texture,
                .uv      = vec2f { location * atlas_alignment } / vec2f { atlas_size },
                .wh      = vec2f { size } / vec2f { atlas_size },
                .binding = 0
            };

            return st;
        }


        template <typename Pixel> void store_at(const image<Pixel>& img, subtexture& where) {
            VE_DEBUG_ASSERT(
                vec2ui { (where.wh * vec2f { atlas_size }) + vec2f { epsilon } } == img.size,
                "Image dimensions do not match dimensions of pre-allocated texture storage."
            );

            auto block_size = unaligned_to_blockpos(img.size);
            auto location   = vec2ui { (where.uv * vec2f { atlas_size } / vec2f { (f32) atlas_alignment }) + vec2f { epsilon } };

            texture->write(img, location * atlas_alignment);
        }


        VE_GET_CREF(name);
        VE_GET_CREF(texture);
    private:
        // Can be used to distinguish texture coordinates from block coordinates.
        using blockpos = vec2ui;


        std::string name;

        vec2ui atlas_size;
        u32 atlas_alignment;

        shared<gfxapi::texture> texture;
        hash_set<vec2ui> occupied_blocks;


        blockpos unaligned_to_blockpos(const vec2ui& vec) {
            return (vec + atlas_alignment - 1u) / atlas_alignment;
        }


        blockpos find_storage(const blockpos& size) const {
            const blockpos blocks = atlas_size / atlas_alignment;

            // TODO: Optimize this. Start at the far edge instead so we can jump by size rather than by one if the block is taken.
            for (u32 x = 0; x <= blocks.x - size.x; ++x) {
                for (u32 y = 0; y <= blocks.y - size.y; ++y) {
                    for (u32 dx = 0; dx < size.x; ++dx) {
                        for (u32 dy = 0; dy < size.y; ++dy) {
                            if (occupied_blocks.contains(vec2ui { x + dx, y + dy })) goto try_next;
                        }
                    }

                    return vec2ui { x, y };
                    try_next:;
                }
            }

            throw atlas_full_error { ve::cat("Atlas has no space for texture that would occupy ", size.x, " x ", size.y, " blocks.") };
        }


        template <bool Occupied> void mark_storage(const blockpos& where, const blockpos& size) {
            for (u32 x = where.x; x < where.x + size.x; ++x) {
                for (u32 y = where.y; y < where.y + size.y; ++y) {
                    if constexpr (Occupied) occupied_blocks.emplace(x, y);
                    else occupied_blocks.erase(vec2ui { x, y });
                }
            }
        }
    };
}