#include <VEDemoGame/game.hpp>


// Invoked by the engine (See VoxelEngine/engine.cpp).
namespace ve::game_callbacks {
    void pre_init (void) { demogame::game::pre_init();  }
    void post_init(void) { demogame::game::post_init(); }
    void pre_loop (void) { demogame::game::pre_loop();  }
    void post_loop(void) { demogame::game::post_loop(); }
    void pre_exit (void) { demogame::game::pre_exit();  }
    void post_exit(void) { demogame::game::post_exit(); }
}


namespace demogame {
    
}