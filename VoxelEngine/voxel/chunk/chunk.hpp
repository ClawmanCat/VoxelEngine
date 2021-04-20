#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/tile/tile_provider.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/traits/maybe_const.hpp>


namespace ve {
    class chunk : public tile_provider<chunk> {
    public:
        [[nodiscard]] const tile_data& get(const vec3i& where) const {
            return data[flatten(where, (i32) voxel_settings::chunk_size)];
        }
    
        void set(const vec3i& where, const tile_data& td) {
            data[flatten(where, (i32) voxel_settings::chunk_size)] = td;
        }
        
        template <typename Pred> void foreach(const Pred& pred) {
            foreach_impl<Pred, false>(*this, pred);
        }
    
        template <typename Pred> void foreach(const Pred& pred) const {
            foreach_impl<Pred, true>(*this, pred);
        }
        
    private:
        // TODO: Optimize for caching. Use NxNxN cubes and prefetch on loop?
        std::array<tile_data, cube(voxel_settings::chunk_size)> data;
    
    
        // Template for both const and non-const versions.
        template <typename Pred, bool as_const> requires std::is_invocable_v<Pred, const vec3i&, u32, meta::maybe_const<as_const, tile_data&>>
        static void foreach_impl(meta::maybe_const<as_const, chunk&> self, const Pred& pred)  {
            // If pred returns bool, break the iteration if it returns false.
            constexpr bool returns_break = std::is_invocable_r_v<bool, Pred, const vec3i&, u32, meta::maybe_const<as_const, tile_data&>>;
        
        
            u32 index = 0;
            vec3i pos;
        
            for (pos.x = 0; pos.x < (i32) voxel_settings::chunk_size; ++pos.x) {
                for (pos.y = 0; pos.y < (i32) voxel_settings::chunk_size; ++pos.y) {
                    for (pos.z = 0; pos.z < (i32) voxel_settings::chunk_size; ++pos.z) {
                        if constexpr (returns_break) { if (!pred(pos, index, self.data[index])) return; }
                        else pred(pos, index, self.data[index]);
                    
                        ++index;
                    }
                }
            }
        }
    };
}