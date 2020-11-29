#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/engine.hpp>

#include <array>


#define VE_IMPL_ENGINE_EVENT(name) \
struct engine_##name##_event : public ve::events::event {};


namespace ve {
    VE_IMPL_ENGINE_EVENT(pre_init)
    VE_IMPL_ENGINE_EVENT(post_init)
    VE_IMPL_ENGINE_EVENT(pre_loop)
    VE_IMPL_ENGINE_EVENT(post_loop)
    VE_IMPL_ENGINE_EVENT(pre_exit)
    VE_IMPL_ENGINE_EVENT(post_exit)
    VE_IMPL_ENGINE_EVENT(delayed_exit)
    VE_IMPL_ENGINE_EVENT(immediate_exit)
    VE_IMPL_ENGINE_EVENT(uncaught_error)
    
    
    struct engine_state_change_event : public ve::events::event {
        engine_state_change_event(engine::state prev, engine::state current)
            : old_state(prev), new_state(current)
        {}
        
        engine::state old_state, new_state;
    };
}