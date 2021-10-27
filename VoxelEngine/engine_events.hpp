#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    struct engine_pre_init_event  {};
    struct engine_post_init_event {};

    struct engine_pre_loop_event  { u64 tick; };
    struct engine_post_loop_event { u64 tick; };

    struct engine_exit_requested_event { i32 code; };
    struct engine_pre_exit_event  { i32 code; };
    struct engine_post_exit_event { i32 code; };
}