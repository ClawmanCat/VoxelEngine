#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/graphics/presentation/window_registry.hpp>


// Initialization of aggregate with members in base class.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"


// TODO: Refactor this file. There is a lot of code duplication for different event types.
namespace ve {
    input_manager& input_manager::instance(void) {
        static input_manager i { };
        return i;
    }


    void input_manager::update(u64 tick) {
        // Current state will be updated from event handlers.
        prev_mouse_state = current_mouse_state;


        auto now = steady_clock::now();
        SDL_Event event;
        std::optional<SDL_Event> exit_event;

        while (SDL_PollEvent(&event)) {
            auto window_of = [] (const auto& sub_event) {
                return gfx::window_registry::instance().get_window(SDL_GetWindowFromID(sub_event.windowID));
            };


            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    handle_key_event(event, window_of(event.key), tick, now);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    handle_mouse_button_event(event, window_of(event.button), tick, now);
                    break;
                case SDL_MOUSEMOTION:
                    handle_mouse_move_event(event, window_of(event.motion), tick, now);
                    break;
                case SDL_MOUSEWHEEL:
                    handle_mouse_wheel_event(event, window_of(event.wheel), tick, now);
                    break;
                case SDL_WINDOWEVENT:
                    handle_window_event(event, window_of(event.window), tick, now);
                    break;
                case SDL_QUIT:
                    // Delay quit event until after all other events have been handled.
                    exit_event = event;
                    break;
                default:
                    VE_DEBUG_ONLY(
                        // Only log unhandled event types once.
                        static hash_set<u32> notified_event_types;

                        if (notified_event_types.insert(event.type).second) {
                            VE_LOG_DEBUG(cat("Unhandled SDL event type: ", event.type));
                        }
                    );

                    break;
            }


            if (auto it = custom_handlers.find((SDL_EventType) event.type); it != custom_handlers.end()) {
                for (auto& handler : it->second) handler(event);
            }
        }


        // Note: key_hold events are dispatched manually, since they have no corresponding SDL event.
        if (auto* keyboard_window_ptr = SDL_GetKeyboardFocus(); keyboard_window_ptr) {
            auto* keyboard_window = gfx::window_registry::instance().get_window(keyboard_window_ptr);

            for (const auto& [key, state] : keyboard_state) {
                if (state.is_down) dispatch_event(key_hold_event { keyboard_window, state });
            }
        }


        // Note: same for mouse_[move|wheel|drag]_end events.
        // TODO: What is the behaviour here when moving between two windows during an event?
        // TODO: What is the behaviour when a move|wheel|drag is ended outside any SDL window?
        // (If these events don't already end under such conditions, they should probably be forced to do so manually.)
        if (auto* mouse_window_ptr = SDL_GetMouseFocus(); mouse_window_ptr) {
            auto* mouse_window = gfx::window_registry::instance().get_window(mouse_window_ptr);

            if (current_mouse_state.position == prev_mouse_state.position) {
                if (current_mouse_move) {
                    dispatch_event(mouse_move_end_event { mouse_window, current_mouse_move->begin_state, current_mouse_state });
                    current_mouse_move = std::nullopt;
                }
            }

            if (current_mouse_state.wheel_position == prev_mouse_state.wheel_position) {
                if (current_mouse_wheel_move) {
                    dispatch_event(mouse_wheel_move_end_event { mouse_window, current_mouse_wheel_move->begin_state, current_mouse_state });
                    current_mouse_wheel_move = std::nullopt;
                }
            }

            // Drag ends on click release, not on mouse movement end.
            for (mouse_button button : magic_enum::enum_values<mouse_button>()) {
                auto& current_drag = current_mouse_drag[(u32) button];

                if (current_drag && !current_mouse_state.buttons[(u32) button].is_down) {
                    dispatch_event(mouse_drag_end_event { mouse_window, button, current_drag->begin_state, current_mouse_state });
                    current_drag = std::nullopt;
                }
            }
        }


        if (exit_event) dispatch_event(exit_requested_event { });
    }


    input_manager::custom_handler_handle input_manager::add_custom_handler(SDL_EventType type, custom_event_handler&& handler) {
        auto& handlers_for_t = custom_handlers[type];

        auto it = handlers_for_t.emplace(handlers_for_t.end(), std::move(handler));
        return { type, it };
    }


    void input_manager::remove_custom_handler(custom_handler_handle handle) {
        if (auto it = custom_handlers.find(handle.type); it != custom_handlers.end()) {
            it->second.erase(handle.it);
            if (it->second.empty()) custom_handlers.erase(it);
        }
    }


    const key_state& input_manager::get_key_state(SDL_Keycode key) const {
        return get_mutable_key_state(key);
    }


    key_state& input_manager::get_mutable_key_state(SDL_Keycode key) const {
        if (auto it = keyboard_state.find(key); it != keyboard_state.end()) [[likely]] {
            return it->second;
        }

        auto [it, success] = keyboard_state.emplace(
            key,
            key_state {
                .key              = key,
                .mods             = SDL_Keymod::KMOD_NONE,
                .last_change      = steady_clock::time_point { },
                .last_change_tick = 0,
                .is_down          = (bool) SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(key)]
            }
        );

        return it->second;
    }


    bool input_manager::is_key_pressed(SDL_Keycode key) const {
        return get_key_state(key).is_down;
    }


    const mouse_button_state& input_manager::get_mouse_button_state(mouse_button button) const {
        return current_mouse_state.buttons[(u32) button];
    }


    mouse_button_state& input_manager::get_mutable_mouse_button_state(mouse_button button) {
        return current_mouse_state.buttons[(u32) button];
    }


    bool input_manager::is_mouse_button_pressed(mouse_button button) const {
        return get_mouse_button_state(button).is_down;
    }


    keymods input_manager::get_current_keymods(void) const {
        #define ve_impl_kmod(key, mod) if (is_key_pressed(key)) mods |= mod


        // Note: SDL provides no proper way to distinguish AltGr from LCtrl + RAlt, so this mod is never set here.
        keymods mods = KMOD_NONE;
        ve_impl_kmod(SDLK_LCTRL,        KMOD_LCTRL);
        ve_impl_kmod(SDLK_RCTRL,        KMOD_RCTRL);
        ve_impl_kmod(SDLK_LSHIFT,       KMOD_LSHIFT);
        ve_impl_kmod(SDLK_RSHIFT,       KMOD_RSHIFT);
        ve_impl_kmod(SDLK_LALT,         KMOD_LALT);
        ve_impl_kmod(SDLK_RALT,         KMOD_RALT);
        ve_impl_kmod(SDLK_LGUI,         KMOD_LGUI);
        ve_impl_kmod(SDLK_RGUI,         KMOD_RGUI);
        ve_impl_kmod(SDLK_CAPSLOCK,     KMOD_CAPS);
        ve_impl_kmod(SDLK_NUMLOCKCLEAR, KMOD_NUM);


        return mods;
    }


    void input_manager::handle_key_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now) {
        auto& old_state = get_mutable_key_state(event.key.keysym.sym);

        key_state new_state = old_state;
        new_state.is_down   = (event.type == SDL_KEYDOWN);


        if (new_state.is_down != old_state.is_down) {
            new_state.last_change      = now;
            new_state.last_change_tick = tick;

            if (new_state.is_down) new_state.mods = event.key.keysym.mod;
        }


        if (new_state.is_down) {
            if (!old_state.is_down) {
                // Unpressed to pressed, this was a keydown event.
                dispatch_event(key_down_event { window, old_state, new_state });
            } else {
                // Still pressed, this is a key typed event.
                dispatch_event(key_type_event { window, new_state });
            }
        } else if (old_state.is_down) {
            // Pressed to unpressed, this was a keyup event.
            dispatch_event(key_up_event { window, old_state, new_state });
        }


        old_state = new_state;
    }


    void input_manager::handle_mouse_button_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now) {
        auto& old_state = get_mutable_mouse_button_state(button_from_sdl(event.button.button));

        mouse_button_state new_state = old_state;
        new_state.is_down = (event.type == SDL_MOUSEBUTTONDOWN);


        if (new_state.is_down != old_state.is_down) {
            new_state.last_change      = now;
            new_state.last_change_tick = tick;

            if (new_state.is_down) new_state.mods = get_current_keymods();
        }


        if (new_state.is_down) {
            if (!old_state.is_down) {
                // Unpressed to pressed, this was a button press event.
                dispatch_event(button_press_event { window, old_state, new_state });
            } else {
                // Still pressed, this is a button hold event.
                dispatch_event(button_hold_event { window, new_state });
            }
        } else {
            // Pressed to unpressed, this way a button release event.
            dispatch_event(button_release_event { window, old_state, new_state });
        }


        old_state = new_state;
    }


    void input_manager::handle_mouse_move_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now) {
        current_mouse_state.position += vec2i { event.motion.x, event.motion.y };


        if (!current_mouse_move) {
            // Begin new mouse movement tracker.
            current_mouse_move = ongoing_mouse_event { current_mouse_state };
            dispatch_event(mouse_move_start_event { window, current_mouse_state });
        }

        dispatch_event(mouse_moved_event { window, current_mouse_move->begin_state, prev_mouse_state, current_mouse_state });


        for (mouse_button button : magic_enum::enum_values<mouse_button>()) {
            auto& current_drag = current_mouse_drag[(u32) button];

            if (current_mouse_state.buttons[(u32) button].is_down) {
                if (!current_drag) {
                    // Begin new mouse drag tracker.
                    current_drag = ongoing_mouse_event { current_mouse_state };
                    dispatch_event(mouse_drag_start_event { window, button, current_mouse_state });
                }

                dispatch_event(mouse_drag_event { window, button, current_drag->begin_state, prev_mouse_state, current_mouse_state });
            }
        }
    }


    void input_manager::handle_mouse_wheel_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now) {
        current_mouse_state.wheel_position += event.wheel.y;


        if (!current_mouse_wheel_move) {
            // Begin new mouse wheel movement tracker.
            current_mouse_wheel_move = ongoing_mouse_event { current_mouse_state };
            dispatch_event(mouse_wheel_move_start_event { window, current_mouse_state });
        }

        dispatch_event(mouse_wheel_moved_event { window, current_mouse_wheel_move->begin_state, prev_mouse_state, current_mouse_state });
    }


    void input_manager::handle_window_event(SDL_Event& event, gfx::window* window, u64 tick, steady_clock::time_point now) {
        // Note: window_opened_event, first_window_opened_event and last_window_closed event are not handled here,
        // since they don't have an associated SDL event. They are triggered from the window registry with trigger_event.
        auto  new_state = per_window_data { .size = window->get_window_size(), .location = window->get_location() };
        // Set to current value if we don't have information about the old size.
        auto& old_state = window_data.try_emplace(window->get_handle(), new_state).first->second;


        switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:        return dispatch_event(window_closed_event { window });
            case SDL_WINDOWEVENT_MAXIMIZED:    return dispatch_event(window_maximized_event { window });
            case SDL_WINDOWEVENT_MINIMIZED:    return dispatch_event(window_minimized_event { window });
            case SDL_WINDOWEVENT_RESTORED:     return dispatch_event(window_restored_event { window });
            case SDL_WINDOWEVENT_HIDDEN:       return dispatch_event(window_hidden_event { window });
            case SDL_WINDOWEVENT_SHOWN:        return dispatch_event(window_shown_event { window });
            case SDL_WINDOWEVENT_EXPOSED:      return dispatch_event(window_exposed_event { window });
            case SDL_WINDOWEVENT_FOCUS_GAINED: return dispatch_event(window_gain_keyboard_focus_event { window });
            case SDL_WINDOWEVENT_FOCUS_LOST:   return dispatch_event(window_lose_keyboard_focus_event { window });
            case SDL_WINDOWEVENT_ENTER:        return dispatch_event(window_gain_mouse_focus_event { window });
            case SDL_WINDOWEVENT_LEAVE:        return dispatch_event(window_lose_mouse_focus_event { window });

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                dispatch_event(window_resized_event { window, old_state.size, new_state.size });
                old_state.size = new_state.size;
                return;

            case SDL_WINDOWEVENT_MOVED:
                dispatch_event(window_moved_event { window, old_state.location, new_state.location });
                old_state.location = new_state.location;
                return;

            default:
                VE_DEBUG_ONLY(
                    // Only log unhandled event types once.
                    static hash_set<u32> notified_event_types;

                    if (notified_event_types.insert(event.type).second) {
                        VE_LOG_DEBUG(cat("Unhandled SDL event type: ", event.type));
                    }
                );

                return;
        }
    }
}


#pragma clang diagnostic pop