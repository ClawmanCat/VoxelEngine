#pragma once

#include <VoxelEngine/voxel/voxel.hpp>


namespace demo_game::tiles {
    inline const ve::voxel::tile* store_and_register(const ve::voxel::tile::arguments& args) {
        static std::vector<ve::unique<ve::voxel::tile>> storage { };

        auto& ptr = storage.emplace_back(ve::make_unique<ve::voxel::tile>(args));
        ve::voxel::voxel_settings::get_tile_registry().register_tile(ptr.get());

        return ptr.get();
    }


    inline const ve::voxel::tile* TILE_GRASS = store_and_register(ve::voxel::tile::arguments {
        .name         = "grass",
        .texture_name = "hd_grass"
    });

    inline const ve::voxel::tile* TILE_STONE = store_and_register(ve::voxel::tile::arguments {
        .name         = "stone",
        .texture_name = "hd_stone"
    });

    inline const ve::voxel::tile* TILE_BRICK = store_and_register(ve::voxel::tile::arguments {
        .name         = "brick",
        .texture_name = "brick_wall"
    });

    inline const ve::voxel::tile* TILE_EMISSIVE = store_and_register(ve::voxel::tile::arguments {
        .name         = "emissive",
        .texture_name = "emissive_stone"
    });
}