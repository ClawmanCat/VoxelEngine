#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/voxel/tile/tile_registry.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>


test_result test_main(void) {
    ve::voxel::tile_registry registry { };

    // Generate some tiles.
    std::vector<ve::unique<ve::voxel::tile>> tiles;
    for (std::size_t i = 0; i < 1000; ++i) {
        tiles.emplace_back(ve::make_unique<ve::voxel::tile>(ve::voxel::tile::arguments { .name = "Test" }));
        registry.register_tile(tiles.back().get());
    }

    // Create some tombstones.
    for (std::size_t i = 999; i > 37; i -= 37) {
        registry.unregister_tile(tiles[i].get());
        tiles.erase(tiles.begin() + i);
    }

    // For every tile, convert it to a tilestate and back, and assert we get the same thing back.
    test_result result = VE_TEST_SUCCESS;

    for (const auto& tile : tiles) {
        auto td = registry.get_state(tile.get(), 0);
        const auto* result_tile = registry.get_tile_for_state(td);

        if (result_tile != tile.get()) result |= VE_TEST_FAIL("Tile registry returned incorrect value ", result_tile, " for tile ", tile.get());
    }

    return result;
}