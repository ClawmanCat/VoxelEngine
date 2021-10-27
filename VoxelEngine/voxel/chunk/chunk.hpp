#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile_provider.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/friend.hpp>


namespace ve::voxel {
    class chunk : public tile_provider<chunk> {
    public:
        constexpr static bool is_bounded(void) {
            return true;
        }

        constexpr static tilepos get_extents(void) {
            return tilepos { voxel_settings::chunk_size };
        }


        const tile_data& get_data(const tilepos& where) const {
            return data[flatten(where, (tilepos::value_type) voxel_settings::chunk_size)];
        }


        void set_data(const tilepos& where, const tile_data& td) {
            data[flatten(where, (tilepos::value_type) voxel_settings::chunk_size)] = td;
        }


        template <typename Pred> requires std::is_invocable_v<Pred, tilepos, tile_data&>
        void foreach(Pred pred) {
            foreach_common(pred, *this);
        }

        template <typename Pred> requires std::is_invocable_v<Pred, tilepos, const tile_data&>
        void foreach(Pred pred) const {
            foreach_common(pred, *this);
        }
    private:
        friend struct chunk_access;

        // TODO: Use a more cache-optimal data layout.
        std::array<tile_data, cube(voxel_settings::chunk_size)> data;


        static void foreach_common(auto pred, auto& self) {
            std::size_t i = 0;

            for (u32 x = 0; x < voxel_settings::chunk_size; ++x) {
                for (u32 y = 0; y < voxel_settings::chunk_size; ++y) {
                    for (u32 z = 0; z < voxel_settings::chunk_size; ++z) {
                        std::invoke(pred, tilepos { x, y, z }, self.data[i++]);
                    }
                }
            }
        }
    };


    ve_friend_interface(chunk_access, chunk, data);
}