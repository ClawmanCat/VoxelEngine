#pragma once

#include <VEDemoGame/core/core.hpp>

#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/graphics/render/texture/aligned_texture_atlas.hpp>



namespace demo_game {
    class game {
    public:
        static void on_pre_init(void)  noexcept;
        static void on_post_init(void) noexcept;
        static void on_pre_loop(void)  noexcept;
        static void on_post_loop(void) noexcept;
        static void on_pre_exit(void)  noexcept;
        static void on_post_exit(void) noexcept;
        
        static void on_actor_id_provided(ve::actor_id id) noexcept;
        [[nodiscard]] static const ve::game_info* get_info(void) noexcept;
        
        [[nodiscard]] static ve::actor_id get_id(void) noexcept {
            return game::id;
        }
    private:
        // Guaranteed to be set by on_actor_id_provided before any other method is called.
        static inline ve::actor_id id = 0;
        
        static inline ve::aligned_texture_atlas<>* atlas = nullptr;
    };
}


namespace ve::game_callbacks {
    // Callbacks will be called by the engine to initialize the game.
    
    // Called before any of the other callbacks to provide the game with an actor id.
    void on_actor_id_provided(ve::actor_id id) {
        demo_game::game::on_actor_id_provided(id);
    }
    
    // Can be called at any point to request game information.
    const ve::game_info* get_game_info(void) {
        return demo_game::game::get_info();
    }
    
    
    // Called before and after engine::on_init, engine::on_loop, engine::on_exit.
    // In the case of on_game_post_exit, the method is called just before std::exit.
    void on_game_pre_init(void)  { demo_game::game::on_pre_init();  }
    void on_game_post_init(void) { demo_game::game::on_post_init(); }
    void on_game_pre_loop(void)  { demo_game::game::on_pre_loop();  }
    void on_game_post_loop(void) { demo_game::game::on_post_loop(); }
    void on_game_pre_exit(void)  { demo_game::game::on_pre_exit();  }
    void on_game_post_exit(void) { demo_game::game::on_post_exit(); }
}