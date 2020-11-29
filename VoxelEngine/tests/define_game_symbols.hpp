#pragma once

#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/dependent/game.hpp>


// Define game symbols in testing mode so we don't get linker errors due to the lack of a linked game.
#ifdef VOXELENGINE_TESTING
    namespace ve::game_callbacks {
        void on_actor_id_provided(ve::actor_id id) {}
    
        const ve::game_info* get_game_info(void) {
            const static ve::game_info info {
                .name         = "Dummy Game Info",
                .description  = { "Dummy game info object used when running VoxelEngine tests." },
                .authors      = { "ClawmanCat" },
                .game_version = { "NoVersion", 0, 0, 0 }
            };
            
            return &info;
        }
    
        void on_game_pre_init(void)  { }
        void on_game_post_init(void) { }
        void on_game_pre_loop(void)  { }
        void on_game_post_loop(void) { }
        void on_game_pre_exit(void)  { }
        void on_game_post_exit(void) { }
    }
#endif