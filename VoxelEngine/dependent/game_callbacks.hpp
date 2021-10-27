#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/dependent_info.hpp>


namespace ve::game_callbacks {
    extern void pre_init(void);
    extern void post_init(void);
    
    extern void pre_loop(void);
    extern void post_loop(void);
    
    extern void pre_exit(void);
    extern void post_exit(void);
    
    extern const game_info* get_info(void);
}