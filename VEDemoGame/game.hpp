#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VEDemoGame/entity/world.hpp>

#include <VoxelEngine/dependent/game.hpp>


namespace demo_game {
    class game {
    public:
        static void on_pre_init(void);
        static void on_post_init(void);
        static void on_pre_loop (ve::u64 tick, microseconds dt);
        static void on_post_loop(ve::u64 tick, microseconds dt);
        static void on_pre_exit(void);
        static void on_post_exit(void);
        
        [[nodiscard]] static const ve::game_info* get_info(void);
        [[nodiscard]] static gfx::window& get_window(void) { return *window; }
        
    //private:
        static ve::shared<gfx::window> window;
        static ve::scene<ve::side::CLIENT> scene;
        static gfx::perspective_camera camera;
        static inline world* world = nullptr;
    };
}


// Callbacks for the game engine.
namespace ve::game_callbacks {
    void on_game_pre_init(void)  { demo_game::game::on_pre_init();  }
    void on_game_post_init(void) { demo_game::game::on_post_init(); }
    void on_game_pre_loop (ve::u64 tick, microseconds dt) { demo_game::game::on_pre_loop (tick, dt); }
    void on_game_post_loop(ve::u64 tick, microseconds dt) { demo_game::game::on_post_loop(tick, dt); }
    void on_game_pre_exit(void)  { demo_game::game::on_pre_exit();  }
    void on_game_post_exit(void) { demo_game::game::on_post_exit(); }
    
    const ve::game_info* get_game_info(void) {
        return demo_game::game::get_info();
    }
}