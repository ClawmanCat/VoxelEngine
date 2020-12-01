#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>
#include <VoxelEngine/graphics/render/texture/texture_atlas.hpp>

#include <optional>
#include <vector>


namespace ve {
    template <std::size_t Align = 32> class aligned_texture_atlas : public texture_atlas {
    public:
        using atlas_id = u64;
        
        
        aligned_texture_atlas(const vec2ui& size = { 2048, 2048 }, u32 mipmap_level = 4) : texture_atlas(size) {
            reserve_texture_storage(
                std::vector<texture_parameter> {
                    { GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST },
                    { GL_TEXTURE_MAG_FILTER, GL_NEAREST },
                    { GL_TEXTURE_WRAP_S,     GL_REPEAT  },
                    { GL_TEXTURE_WRAP_T,     GL_REPEAT  }
                },
                mipmap_level
            );
        }
        
        
        // Adds a texture to the atlas. Returns nullopt if the atlas is full.
        std::optional<atlas_id> add_subtexture(const io::image& img) {
            auto size = align_coords(img.size);
            auto position = find_free_location(size);
            if (!position) return std::nullopt;
            
            store_subtexture(img, dealign_coords(*position));
            
            atlas_id id = next_id++;
            
            texture_location insert_pos = { *position, size };
            subtextures.insert({ id, insert_pos });
            set_occupied<true>(insert_pos);
            
            return id;
        }
        
        
        void remove_subtexture(atlas_id id) {
            auto it = subtextures.find(id);
            if (it == subtextures.end()) return;
            
            texture_location where = it->second;
            subtextures.erase(it);
            
            set_occupied<false>(where);
        }
        
        
        std::optional<subtexture> get_subtexture(atlas_id id) {
            auto it = subtextures.find(id);
            if (it == subtextures.end()) return std::nullopt;
            
            texture_location where = it->second;
            vec2f xy = coords_to_uv(dealign_coords(where.position));
            vec2f zw = xy + coords_to_uv(dealign_coords(where.size));
            
            // Flip zw and xy because OpenGL stores the texture upside down.
            return subtexture { tex, vec4f { zw, xy } };
        }
    private:
        struct texture_location {
            vec2ui position, size;  // In Aligned-Position coordinates.
        };
        
        hash_map<atlas_id, texture_location> subtextures;
        hash_set<vec2ui> occupied_positions;
        atlas_id next_id = 0;
        
        
        template <bool occupied> void set_occupied(const texture_location& where) {
            auto fn = meta::return_if<occupied>(
                [&](const vec2ui& pos) { occupied_positions.insert(pos); },
                [&](const vec2ui& pos) { occupied_positions.erase(pos);  }
            );
            
            for (u32 x = where.position.x; x < where.position.x + where.size.x; ++x) {
                for (u32 y = where.position.y; y < where.position.y + where.size.y; ++y) {
                    fn({ x, y });
                }
            }
        }
        
        
        std::optional<vec2ui> find_free_location(const vec2ui& size) {
            VE_ASSERT(glm::all(size <= this->size));
            
            auto check_free = [&](const vec2ui& pos, const vec2ui& size) {
                for (u32 x = pos.x; x < pos.x + size.x; ++x) {
                    for (u32 y = pos.y; y < pos.y + size.y; ++y) {
                        if (occupied_positions.contains({ x, y })) return false;
                    }
                }
                
                return true;
            };
            
            vec2ui position_limit = align_coords(this->size - size + 1u);
            for (u32 x = 0; x < position_limit.x; ++x) {
                for (u32 y = 0; y < position_limit.y; ++y) {
                    if (check_free({ x, y }, size)) return vec2ui { x, y };
                }
            }
            
            return std::nullopt;
        }
        
        
        constexpr static vec2ui align_coords(const vec2ui& pos) {
            return pos / (u32) Align;
        }
        
        
        constexpr static vec2ui dealign_coords(const vec2ui& pos) {
            return pos * (u32) Align;
        }
    };
}