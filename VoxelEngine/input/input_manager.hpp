#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/key_state.hpp>
#include <VoxelEngine/input/button_state.hpp>
#include <VoxelEngine/input/mouse_button.hpp>
#include <VoxelEngine/input/mouse_state.hpp>
#include <VoxelEngine/utils/meta/immovable.hpp>
#include <VoxelEngine/event/delayed_prioritized_event_dispatcher.hpp>

#include <SDL.h>

#include <shared_mutex>


namespace ve {
    class input_manager {
    public:
        [[nodiscard]] static input_manager& instance(void) noexcept;
        
        void update(u64 tick);
        
        [[nodiscard]] auto& get_dispatcher(void) noexcept { return dispatcher; }
    private:
        template <bool, bool> friend void handle_button_change(input_manager&, const SDL_Event&, u64);
        
        input_manager(void) = default;
        ve_make_immovable;
        
        
        std::shared_mutex mtx;
        hash_map<SDL_Keycode, key_state>     keyboard_state;
        hash_map<mouse_button, button_state> button_states;
        
        // Mouse state for the current tick, the previous tick,
        // the last tick in which the mouse was not moving and the last tick in which the mouse moved.
        mouse_state current, previous, last_stopped_state, last_moving_state;
        mouse_state last_wheel_stopped_state, last_wheel_moving_state;
        
        events::delayed_prioritized_event_dispatcher<false, true> dispatcher;
    };
}