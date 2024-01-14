#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::game_callbacks {
    extern void pre_init (void);
    extern void post_init(void);
    extern void pre_loop (void);
    extern void post_loop(void);
    extern void pre_exit (void);
    extern void post_exit(void);
}