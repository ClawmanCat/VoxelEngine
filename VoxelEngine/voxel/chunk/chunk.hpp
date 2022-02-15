#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile_provider.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/spatial_iterate.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>


namespace ve::voxel {
    class chunk : public tile_provider<chunk> {
    public:
        using data_t = std::array<tile_data, cube(voxel_settings::chunk_size)>;


        constexpr static bool is_bounded(void) {
            return true;
        }

        constexpr static tilepos get_extents(void) {
            return tilepos { voxel_settings::chunk_size };
        }


        const tile_data& get_data(const tilepos& where) const {
            return data[flatten(where, (tilepos::value_type) voxel_settings::chunk_size)];
        }


        tile_data set_data(const tilepos& where, const tile_data& td) {
            auto& old_value = data[flatten(where, (tilepos::value_type) voxel_settings::chunk_size)];

            if (locked) [[unlikely]] {
                pending_changes.emplace_back(where, td);
                return old_value;
            }

            return std::exchange(old_value, td);
        }


        template <typename Pred> requires std::is_invocable_v<Pred, tilepos, const tile_data&>
        void foreach(Pred pred) const {
            foreach_common(pred, *this);
        }


        void toggle_write_lock(bool locked) {
            this->locked = locked;

            if (!locked) {
                for (const auto& [where, data] : pending_changes) {
                    set_data(where, data);
                }

                pending_changes.clear();
            }
        }


        bool has_pending_changes(void) const {
            return !pending_changes.empty();
        }


        void set_chunk_data(const data_t& data) {
            if (locked) [[unlikely]] {
                // TODO: Find a more optimal way to do this.
                std::size_t i = 0;

                pending_changes.clear();
                pending_changes.reserve(cube(voxel_settings::chunk_size));

                spatial_iterate<
                    voxel_settings::chunk_size,
                    voxel_settings::chunk_size,
                    voxel_settings::chunk_size
                >([&] (auto... position) {
                    pending_changes.emplace_back(tilepos { position... }, data[i++]);
                });
            } else {
                this->data = data;
            }
        }


        const data_t& get_chunk_data(void) const {
            return data;
        }


        VE_GET_BOOL_IS(locked);
    private:
        friend struct chunk_access;


        data_t data;

        bool locked = false;
        small_vector<std::pair<tilepos, tile_data>> pending_changes;


        // Mutable version is private to prevent issues with write locks.
        // This method can still be accessed through chunk_access.
        template <typename Pred> requires (
            std::is_invocable_v<Pred, tilepos, tile_data&> &&
            !std::is_invocable_v<Pred, tilepos, const tile_data&>
        ) void foreach(Pred pred) {
            foreach_common(pred, *this);
        }


        static void foreach_common(auto pred, auto& self) {
            std::size_t i = 0;

            spatial_iterate<
                voxel_settings::chunk_size,
                voxel_settings::chunk_size,
                voxel_settings::chunk_size
            >([&] (auto... position) {
                std::invoke(pred, tilepos { position... }, self.data[i++]);
            });
        }
    };


    // Friend access for chunk generators.
    struct chunk_access {
        auto& get_data(chunk& c) { return c.data; }

        template <typename Pred> void foreach(chunk& c, Pred pred) {
            c.foreach(std::move(pred));
        }
    };
}