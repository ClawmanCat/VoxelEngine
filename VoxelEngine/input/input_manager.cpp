#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/utility.hpp>


#define VE_IMPL_EVENT_MAP(from, to) \
case from: dispatcher.dispatch_event(set_event_window(e, to { })); break


namespace ve {
    template <typename Event>
    static Event set_event_window(SDL_Event& sdl_event, Event&& e) {
        auto* window = *graphics::window_registry::instance().get_window(sdl_event.window.windowID);
        e.window = window;
        
        return std::forward<Event>(e);
    }
    
    
    
    
    input_manager& input_manager::instance(void) {
        static input_manager i { };
        return i;
    }
    
    
    
    
    bool input_manager::is_pressed(SDL_KeyCode key) const {
        auto it = keyboard_state.find(key);
        return it != keyboard_state.end() && it->second.down;
    }
    
    
    
    
    bool input_manager::is_pressed(mouse_button button) const {
        return mouse_current_state.buttons[(u32) button].down;
    }
    
    
    
    
    const key_state& input_manager::get_state(SDL_Keycode key) const {
        auto it = keyboard_state.find(key);
        if (it != keyboard_state.end()) return it->second;
    
        auto [new_it, success] = keyboard_state.insert({ key, key_state {
            .key       = key,
            .down      = false,
            .mods      = SDL_Keymod::KMOD_NONE,
            .last_down = { },
            .last_up   = { }
        }});
        
        return new_it->second;
    }
    
    
    
    
    const mouse_state& input_manager::get_state(recorded_time when) const {
        switch (when) {
            case recorded_time::CURRENT:            return mouse_current_state;
            case recorded_time::PREVIOUS:           return mouse_last_state;
            case recorded_time::MOUSE_LAST_MOVING:  return mouse_history.last_moving_state;
            case recorded_time::MOUSE_LAST_STOPPED: return mouse_history.last_stopped_state;
            case recorded_time::WHEEL_LAST_MOVING:  return mousewheel_history.last_moving_state;
            case recorded_time::WHEEL_LAST_STOPPED: return mousewheel_history.last_stopped_state;
            default: VE_UNREACHABLE;
        }
    }
    
    
    
    
    bool input_manager::is_mouse_moving(void) const {
        return mouse_current_state.position != mouse_last_state.position;
    }
    
    
    
    
    vec2i input_manager::tick_mouse_delta(void) const {
        return mouse_current_state.position - mouse_last_state.position;
    }
    
    
    
    
    vec2i input_manager::total_mouse_delta(void) const {
        return mouse_current_state.position - mouse_history.last_stopped_state.position;
    }
    
    
    
    
    bool input_manager::is_wheel_moving(void) const {
        return mouse_current_state.wheel_position != mouse_last_state.wheel_position;
    }
    
    
    
    
    i32 input_manager::tick_wheel_delta(void) const {
        return mouse_current_state.wheel_position - mouse_last_state.wheel_position;
    }
    
    
    
    
    i32 input_manager::total_wheel_delta(void) const {
        return mouse_current_state.wheel_position - mousewheel_history.last_stopped_state.wheel_position;
    }
    
    
    
    
    
    void input_manager::set_mouse_capture(bool enabled) {
        SDL_SetRelativeMouseMode((SDL_bool) enabled);
    }
    
    
    
    
    void input_manager::update(u64 tick) {
        dispatcher.dispatch_event(input_processing_begin_event { });
        dispatcher.dispatch_all();
        
        
        mouse_last_state = mouse_current_state;
        
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN: {
                    handle_keyboard_change<true>(e, tick);
                    break;
                }
                
                case SDL_KEYUP: {
                    handle_keyboard_change<false>(e, tick);
                    break;
                }
                
                case SDL_MOUSEBUTTONDOWN: {
                    // Mouse events are only fired after processing all SDL events,
                    // so all events will have the same mouse state.
                    mouse_button btn = button_from_sdl(e.button.button);
                    
                    mouse_button_state& state = mouse_current_state.buttons[(u32) btn];
                    state.down      = true;
                    state.last_down = { tick, steady_clock::now() };
                    
                    break;
                }
                
                case SDL_MOUSEBUTTONUP: {
                    // Mouse events are only fired after processing all SDL events,
                    // so all events will have the same mouse state.
                    mouse_button btn = button_from_sdl(e.button.button);
    
                    mouse_button_state& state = mouse_current_state.buttons[(u32) btn];
                    state.down    = false;
                    state.last_up = { tick, steady_clock::now() };
    
                    break;
                }
                
                case SDL_MOUSEMOTION: {
                    // Mouse events are only fired after processing all SDL events,
                    // so all events will have the same mouse state.
                    mouse_current_state.position += vec2i { e.motion.xrel, e.motion.yrel };
                    
                    break;
                }
                
                case SDL_MOUSEWHEEL: {
                    // Mouse events are only fired after processing all SDL events,
                    // so all events will have the same mouse state.
                    mouse_current_state.wheel_position += e.wheel.y;
                    
                    break;
                }
                
                case SDL_WINDOWEVENT: {
                    switch (e.window.event) {
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_MAXIMIZED,    window_maximized_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_MINIMIZED,    window_minimized_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_RESTORED,     window_restored_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_HIDDEN,       window_hidden_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_SHOWN,        window_shown_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_EXPOSED,      window_exposed_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_FOCUS_GAINED, window_gain_keyboard_focus_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_FOCUS_LOST,   window_lose_keyboard_focus_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_ENTER,        window_gain_mouse_focus_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_LEAVE,        window_lose_mouse_focus_event);
                        VE_IMPL_EVENT_MAP(SDL_WINDOWEVENT_CLOSE,        window_closed_event);
                        
                        case SDL_WINDOWEVENT_RESIZED: {
                            auto& old_size = window_sizes[e.window.windowID];
                            auto  new_size = vec2ui { e.window.data1, e.window.data2 };
                            
                            dispatcher.dispatch_event(set_event_window(
                                e,
                                window_resize_event {
                                    .old_size = std::exchange(old_size, new_size),
                                    .new_size = new_size
                                }
                            ));
                            
                            break;
                        }
                        
                        case SDL_WINDOWEVENT_MOVED: {
                            auto& old_pos = window_positions[e.window.windowID];
                            auto  new_pos = vec2ui { e.window.data1, e.window.data2 };
                            
                            dispatcher.dispatch_event(set_event_window(
                                e,
                                window_move_event {
                                    .old_position = std::exchange(old_pos, new_pos),
                                    .new_position = new_pos
                                }
                            ));
                            
                            break;
                        }
                    }
                    
                    break;
                }
                
                case SDL_QUIT: {
                    dispatcher.dispatch_event(last_window_closed_event { });
                    break;
                }
            }
        }
        
    
        // Dispatch remaining keyboard events.
        for (const auto& [key, state] : keyboard_state) {
            if (state.down) dispatcher.dispatch_event(set_event_window(e, key_hold_event { .state = state }));
        }
        
        
        // Dispatch mouse events.
        handle_mouse_buttons(tick);
        
        handle_mouse_motion<false>(tick);
        handle_mouse_motion<true>(tick);
        
        
        // Actually call the event handlers.
        // This ensures all handlers will see the same, final state if they poll the input manager.
        dispatcher.dispatch_all();
        
        dispatcher.dispatch_event(input_processing_end_event { });
        dispatcher.dispatch_all();
    }
    
    
    
    
    template <bool is_down>
    void input_manager::handle_keyboard_change(SDL_Event& e, u64 tick) {
        using event_type = std::conditional_t<is_down, key_down_event, key_up_event>;
        auto  time_field = meta::pick<is_down>(&key_state::last_down, &key_state::last_up);
    
        // Check if this is actually a key down or a key typed event.
        if (!is_down || e.key.repeat == 0) {
            key_state& stored_state = keyboard_state[e.key.keysym.sym];
            key_state  old_state    = stored_state;
        
            stored_state.down        = is_down;
            stored_state.mods        = e.key.keysym.mod;
            stored_state.*time_field = { tick, steady_clock::now() };
        
        
            dispatcher.dispatch_event(set_event_window(e, event_type {
                .old_state = old_state,
                .new_state = stored_state
            }));
        } else {
            dispatcher.dispatch_event(set_event_window(e, key_typed_event {
                .state = keyboard_state[e.key.keysym.sym]
            }));
        }
    }
    
    template void input_manager::handle_keyboard_change<true>(SDL_Event& e, u64 tick);
    template void input_manager::handle_keyboard_change<false>(SDL_Event& e, u64 tick);
    
    
    
    
    template <bool is_wheel>
    void input_manager::handle_mouse_motion(u64 tick) {
        using move_start_event = std::conditional_t<is_wheel, mousewheel_move_start_event, mouse_move_start_event>;
        using move_end_event   = std::conditional_t<is_wheel, mousewheel_move_end_event, mouse_move_end_event>;
        using move_hold_event  = std::conditional_t<is_wheel, mousewheel_moved_event, mouse_moved_event>;
        
        auto position = meta::pick<is_wheel>(&mouse_state::wheel_position, &mouse_state::position);
        auto history  = is_wheel ? mousewheel_history : mouse_history;
        
        
        const bool moved_this_tick = mouse_current_state.*position != mouse_last_state.*position;
        const bool moved_prev_tick = history.last_stopped_tick < history.last_moving_tick;
    
    
        if (!moved_prev_tick && moved_this_tick) {
            dispatcher.dispatch_event(move_start_event {
                .old_state = mouse_last_state,
                .new_state = mouse_current_state
            });
        } else if (moved_prev_tick && !moved_this_tick) {
            dispatcher.dispatch_event(move_end_event {
                .begin_state = mouse_history.last_stopped_state,
                .old_state   = mouse_last_state,
                .new_state   = mouse_current_state
            });
        }
    
    
        if (moved_this_tick) {
            dispatcher.dispatch_event(move_hold_event {
                .begin_state = mouse_history.last_stopped_state,
                .old_state   = mouse_last_state,
                .new_state   = mouse_current_state
            });
        
            history.last_moving_tick  = tick;
            history.last_moving_state = mouse_current_state;
        } else {
            history.last_stopped_tick  = tick;
            history.last_stopped_state = mouse_current_state;
        }
    }
    
    template void input_manager::handle_mouse_motion<true>(u64);
    template void input_manager::handle_mouse_motion<false>(u64);
    
    
    
    
    void input_manager::handle_mouse_buttons(u64 tick) {
        for (const auto& [last, current] : views::zip(mouse_last_state.buttons, mouse_current_state.buttons)) {
            if (!last.down && current.down) {
                dispatcher.dispatch_event(mouse_down_event {
                    .old_state = mouse_last_state,
                    .new_state = mouse_current_state,
                    .button    = current.button
                });
            } else if (last.down && !current.down) {
                dispatcher.dispatch_event(mouse_up_event {
                    .old_state = mouse_last_state,
                    .new_state = mouse_current_state,
                    .button    = current.button
                });
            }
        
            if (current.down) {
                dispatcher.dispatch_event(mouse_hold_event {
                    .state  = mouse_current_state,
                    .button = current.button
                });
            }
        }
    }
}