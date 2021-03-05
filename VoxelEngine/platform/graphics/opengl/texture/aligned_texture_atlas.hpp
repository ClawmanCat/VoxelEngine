#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_atlas.hpp>


namespace ve::graphics {
    // Atlas optimized for textures that have lengths of some multiple of 'Align'.
    // Other textures may still be stored in this atlas, but may cause wasted space.
    template <std::size_t W = 4096, std::size_t H = 4096, std::size_t Align = 32> requires (H % Align == 0 && W % Align == 0)
    class aligned_texture_atlas : public texture_atlas<aligned_texture_atlas<H, W, Align>> {
    public:
        explicit aligned_texture_atlas(texture_format fmt = texture_format::RGBA8, const texture_parameters& params = {}) {
            tex = std::make_shared<texture>(
                detail::initialize_texture({ W, H }, fmt, params)
            );
        }
        
        ve_move_only(aligned_texture_atlas);
    
    
        void on_actor_destroyed(actor_id id) {
            auto it = position_owners.find(id);
            if (it == position_owners.end()) return;
            
            for (const auto& pos : it->second) {
                // Position is not guaranteed to be occupied, but if it is, this is the owner.
                // (Texture erasing does not remove from the owner list, but inserting a new texture at the location overwrites it.)
                if (occupied_positions.erase(pos)) {
                    if (position_less_than(pos, free_space_start)) free_space_start = pos;
                }
            }
            
            position_owners.erase(it);
        }
    
    
        expected<subtexture> add_texture(const io::image& img, ve_default_actor(owner)) {
            vec2ui texture_position;
            vec2ui new_free_space_start;
            
            // Find first point that has a free area of at least the dimensions of the image.
            const vec2ui search_begin_position = { 0, free_space_start.y };
            
            bool found_position = !iterate_space(
                search_begin_position,
                vec2ui { W, H } - search_begin_position - img.size,
                [&](const vec2ui& pos) {
                    for (u32 x = pos.x; x < pos.x + img.size.x; x += Align) {
                        for (u32 y = pos.y; y < pos.y + img.size.y; y += Align) {
                            if (occupied_positions.contains({ x, y })) return true; // Keep searching.
                            else new_free_space_start = { x, y };
                        }
                    }
                    
                    texture_position = pos;
                    return false; // Break
                }
            );
            
            // Texture atlas is full. (At least for textures of this size and larger.)
            if (!found_position) return make_unexpected("Atlas does not have space for texture.");
            
            
            detail::store_texture_data(*tex, img, texture_position);
            
            
            // Mark location of new image as occupied.
            auto& owner_vec = position_owners[owner];
            
            iterate_space(
                texture_position,
                img.size,
                [&](const vec2ui& pos) {
                    occupied_positions.insert(pos);
                    owner_vec.push_back(pos);
                }
            );
            
            free_space_start = new_free_space_start;
            
            
            return subtexture {
                .tex  = tex,
                .uv   = normalize_position(texture_position),
                .size = normalize_position(img.size)
            };
        }
    
    
        void remove_texture(const subtexture& tex) {
            auto next = [&](const vec2ui& pos) {
                return (pos.x + Align >= W)
                    ? vec2ui { 0, pos.y + Align }
                    : vec2ui { pos.x + Align, pos.y };
            };
            
            iterate_space(
                denormalize_position(tex.uv),
                denormalize_position(tex.size),
                [&](const vec2ui& pos) {
                    if (next(pos) == free_space_start) free_space_start = pos;
                    
                    occupied_positions.erase(pos);
                }
            );
        }
    
    
        static u32 quantization(void) {
            return Align;
        }
    
    
        static vec2ui size(void) {
            return { W, H };
        }
        
    private:
        shared<texture> tex;
        hash_set<vec2ui> occupied_positions;
        hash_map<actor_id, std::vector<vec2ui>> position_owners;
        vec2ui free_space_start = { 0, 0 };
        
        
        static bool position_less_than(const vec2ui& a, const vec2ui& b) {
            if (a.y < b.y) return true;
            return (a.y == b.y && a.x < b.x);
        }
        
        
        template <typename Pred>
        static bool iterate_space(const vec2ui& pos, const vec2ui& size, const Pred& pred) {
            for (u32 x = pos.x; x < pos.x + size.x; x += Align) {
                for (u32 y = pos.y; y < pos.y + size.y; y += Align) {
                    if constexpr (std::is_invocable_r_v<bool, Pred, vec2ui>) {
                        // Pred returns bool, break if it returns false.
                        if (!pred(vec2ui { x, y })) return false;
                    } else {
                        pred(vec2ui { x, y });
                    }
                }
            }
            
            return true;
        }
        
        
        static vec2f normalize_position(const vec2ui& pos) {
            return vec2f { pos } / vec2f { W, H };
        }
        
        static vec2ui denormalize_position(const vec2f& pos) {
            return vec2ui { pos * vec2f { W, H } };
        }
    };
}