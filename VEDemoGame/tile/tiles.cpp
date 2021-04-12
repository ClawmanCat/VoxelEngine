#include <VEDemoGame/tile/tiles.hpp>


namespace demo_game {
    ve::simple_tile_storage& get_tile_store(void) {
        static ve::simple_tile_storage instance;
        return instance;
    }
    
    const ve::tile* tile_grass = get_tile_store().emplace<ve::tile>(ve::tile_parameters { .name = "grass" });
    const ve::tile* tile_stone = get_tile_store().emplace<ve::tile>(ve::tile_parameters { .name = "stone" });
}