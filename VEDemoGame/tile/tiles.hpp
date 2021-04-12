#pragma once

#include <VEDemoGame/core/core.hpp>


namespace demo_game {
    // Meyers Singleton is used here to prevent initialization order fiasco w.r.t. tiles.
    extern ve::simple_tile_storage& get_tile_store(void);
    
    extern const ve::tile* tile_grass;
    extern const ve::tile* tile_stone;
}