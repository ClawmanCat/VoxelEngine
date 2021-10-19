#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/tile_provider.hpp>
#include <VoxelEngine/voxel/tile/tiles.hpp>


namespace ve::voxel {
    class voxel_space : public tile_provider<voxel_space> {
    public:
        tile_data& get_data(const tilepos& where) {
            return get_data_common(where, *this);
        }

        const tile_data& get_data(const tilepos& where) const {
            return get_data_common(where, *this);
        }

        bool is_loaded(const tilepos& chunkpos) const {
            return chunks.contains(chunkpos);
        }
    private:
        hash_map<tilepos, unique<chunk>> chunks;


        static decltype(auto) get_data_common(const auto& where, auto& self) {
            tilepos chunkpos = where / voxel_settings::chunk_size;

            if (auto it = self.chunks.find(chunk_pos); it != self.chunks.end()) {
                return (*(it->second))[where - chunkpos];
            } else {
                return tiles::TILE_UNKNOWN;
            }
        }
    };
}