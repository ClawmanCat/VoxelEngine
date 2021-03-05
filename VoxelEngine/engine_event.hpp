#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/engine_state.hpp>


namespace ve {
    struct engine_state_change : public event {
        engine_state old_state, new_state;
    };
    
    struct engine_tick_begin : public event { u64 tick; };
    struct engine_tick_end   : public event { u64 tick; };
    struct engine_exit_begin : public event { };
}