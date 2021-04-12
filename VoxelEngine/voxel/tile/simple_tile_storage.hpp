#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>


namespace ve {
    class simple_tile_storage {
    public:
        template <typename T, typename... Args> requires std::is_base_of_v<tile, T>
        const tile* emplace(Args&&... args) {
            return storage.emplace_back(
                std::make_unique<T>(std::forward<Args>(args)...)
            ).get();
        }
    
    private:
        std::list<unique<tile>> storage;
    };
    
    
    namespace detail {
        extern simple_tile_storage& ve_tile_storage(void);
    }
}