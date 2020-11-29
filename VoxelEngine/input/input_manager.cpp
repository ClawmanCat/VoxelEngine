#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/threading/shared_lock_guard.hpp>
#include <VoxelEngine/utils/meta/if_constexpr.hpp>
#include <VoxelEngine/engine.hpp>

#include <SDL_events.h>

#include <functional>


namespace ve {
    // Generalized handler for mouse & keyboard button presses.
    template <bool key, bool down> static void handle_button_change(input_manager& self, const SDL_Event& e, u64 tick) {
        using state_t = std::conditional_t<key, key_state, button_state>;
        using event_t = std::conditional_t<key, key_press_event<down>, mouse_press_event<down>>;
        
        auto& map = meta::return_if<key>(self.keyboard_state, self.button_states);
        auto  sym = meta::return_if<key>(e.key.keysym.sym, (mouse_button) e.button.button);
        auto  mod = meta::return_if<key>(e.key.keysym.mod, SDL_GetModState());
        auto  kf  = meta::return_if<key>(&key_state::keycode, &button_state::button);
    
        auto it = map.find(sym);
        state_t old_state = (it == map.end()) ? state_t{} : std::move(it->second);
        state_t new_state = old_state;
        
        new_state.*kf     = sym;
        new_state.is_down = down;
        if (down) new_state.mods = mod;
    
    
        auto tick_field = down ? &state_t::last_down_tick : &state_t::last_up_tick;
        auto time_field = down ? &state_t::last_down_time : &state_t::last_up_time;
    
        new_state.*tick_field = tick;
        new_state.*time_field = steady_clock::now();
    
    
        event_t evnt;
        evnt.old_state = old_state;
        evnt.new_state = new_state;
    
        self.get_dispatcher().dispatch_event(std::move(evnt));
        
        
        map.insert_or_assign(it, new_state.*kf, std::move(new_state));
    }
    
    
    [[nodiscard]] input_manager& input_manager::instance(void) noexcept {
        static input_manager i { };
        return i;
    }
    
    
    void input_manager::update(u64 tick) {
        dispatcher.dispatch_event(pre_input_processed_event { });
        dispatcher.process_events();
        
        // Can't poll mouse wheel directly, state is updated by associated event.
        previous = current;
        SDL_GetMouseState(&current.position.x, &current.position.y);
        
        if (current.position != previous.position) last_moving_state = current;
        else last_stopped_state = current;
        
        
        {
            std::lock_guard lock { mtx };
            
            
            for (const auto& [key, state] : keyboard_state) {
                if (!state.is_down) continue;
                
                key_hold_event evnt;
                evnt.state = state;
                
                dispatcher.dispatch_event(std::move(evnt));
            }
            
            for (const auto& [button, state] : button_states) {
                if (!state.is_down) continue;
                
                mouse_hold_event evnt;
                evnt.state = state;
                
                dispatcher.dispatch_event(std::move(evnt));
            }
            
            
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_KEYDOWN: {
                        // SDL_KEYDOWN is also triggered on key hold.
                        if (e.key.repeat == 0) {
                            handle_button_change<true, true>(*this, e, tick);
                        }
    
                        
                        key_typed_event evnt;
                        evnt.state = keyboard_state[e.key.keysym.sym];
    
                        dispatcher.dispatch_event(std::move(evnt));
                        
                        
                        break;
                    }
                    case SDL_KEYUP: {
                        handle_button_change<true, false>(*this, e, tick);
                        break;
                    }
                    case SDL_MOUSEBUTTONDOWN: {
                        handle_button_change<false, true>(*this, e, tick);
                        break;
                    }
                    case SDL_MOUSEBUTTONUP: {
                        handle_button_change<false, false>(*this, e, tick);
                        break;
                    }
                    case SDL_MOUSEWHEEL: {
                        current.wheel_position += e.wheel.y;
                        
                        if (current.wheel_position != previous.wheel_position) last_wheel_moving_state = current;
                        else last_wheel_stopped_state = current;
                        
                        break;
                    }
                    case SDL_WINDOWEVENT: {
                        switch (e.window.event) {
                            case SDL_WINDOWEVENT_MAXIMIZED:    { dispatcher.dispatch_event(window_maximized_event{});           break; }
                            case SDL_WINDOWEVENT_MINIMIZED:    { dispatcher.dispatch_event(window_minimized_event{});           break; }
                            case SDL_WINDOWEVENT_RESTORED:     { dispatcher.dispatch_event(window_restored_event{});            break; }
                            case SDL_WINDOWEVENT_HIDDEN:       { dispatcher.dispatch_event(window_hidden_event{});              break; }
                            case SDL_WINDOWEVENT_SHOWN:        { dispatcher.dispatch_event(window_shown_event{});               break; }
                            case SDL_WINDOWEVENT_EXPOSED:      { dispatcher.dispatch_event(window_exposed_event{});             break; }
                            case SDL_WINDOWEVENT_FOCUS_GAINED: { dispatcher.dispatch_event(window_gain_keyboard_focus_event{}); break; }
                            case SDL_WINDOWEVENT_FOCUS_LOST:   { dispatcher.dispatch_event(window_lose_keyboard_focus_event{}); break; }
                            case SDL_WINDOWEVENT_ENTER:        { dispatcher.dispatch_event(window_gain_mouse_focus_event{});    break; }
                            case SDL_WINDOWEVENT_LEAVE:        { dispatcher.dispatch_event(window_lose_mouse_focus_event{});    break; }
                            case SDL_WINDOWEVENT_SIZE_CHANGED: {
                                static vec2i prev_size = { };
                                
                                dispatcher.dispatch_event(window_resized_event {
                                    std::exchange(prev_size, window_manager::instance().get_window_size()),
                                    prev_size
                                });
                                
                                break;
                            }
                            case SDL_WINDOWEVENT_MOVED: {
                                static vec2i prev_pos   = { };
                                static u32 prev_display = 0;
                                
                                auto current_pos = window_manager::instance().get_window_position();
                                dispatcher.dispatch_event(window_moved_event {
                                    std::exchange(prev_pos, current_pos.position),
                                    prev_pos,
                                    std::exchange(prev_display, current_pos.display),
                                    prev_display
                                });
                                
                                break;
                            }
                        }
                        
                        break;
                    }
                    case SDL_QUIT: {
                        dispatcher.dispatch_event(window_closed_event { });
                        
                        engine::exit();
                        break;
                    }
                }
            }
        }
    
        dispatcher.dispatch_event(post_input_processed_event { });
        dispatcher.process_events();
    }
}