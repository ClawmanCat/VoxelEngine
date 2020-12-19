#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor_id.hpp>

#include <string>
#include <vector>


namespace ve {
    struct game_info {
        std::string name;
        std::vector<std::string> description;
        std::vector<std::string> authors;
        version game_version;
    };
    
    
    namespace game_callbacks {
        // Implemented by the game, called by the engine.
        // All of these are required.
        
        
        // Can be called at any point to request game information.
        extern const game_info* get_game_info(void);
        
        
        // Called before and after engine::on_init, engine::on_loop, engine::on_exit.
        // In the case of on_game_post_exit, the method is called just before std::exit.
        extern void on_game_pre_init(void);
        extern void on_game_post_init(void);
        
        extern void on_game_pre_loop(void);
        extern void on_game_post_loop(void);
        
        extern void on_game_pre_exit(void);
        extern void on_game_post_exit(void);
    }
}