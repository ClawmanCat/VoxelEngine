#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/pack/pack_ops.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/traits/bind.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>


// TODO: Refactor this file.
// TODO: Updating alias should update all associated binding as well.
// TODO: Event handlers should include the key, since aliasing means it might be unknown otherwise.
namespace ve {
    // Represents an input that is either on or off, like a key on the keyboard.
    struct binary_input {
        using key_t = std::variant<SDL_Keycode, mouse_button>;
        key_t input;

        keymods required_mods = KMOD_NONE;
        keymods excluded_mods = KMOD_NONE;

        enum trigger_when_t : u32 { KEY_DOWN = 0b001, KEY_UP = 0b010, KEY_HOLD = 0b100 };
        u32 trigger_when = KEY_HOLD;
        bool requires_mouse_capture = true;


        bool should_trigger(keymods mods, trigger_when_t when) const {
            return (
                (trigger_when  & when) &&
                (input_manager::instance().has_mouse_capture() || !requires_mouse_capture) &&
                (required_mods & mods) == required_mods &&
                (excluded_mods & mods) == 0
            );
        }


        struct handler_args { keymods mods; trigger_when_t when; gfx::window* window; };
        using handler_t = std::function<void(const handler_args& args)>;
    };


    // Represents an input that includes some movement, like a mouse drag.
    struct motion_input {
        struct key_t {
            enum motion_type_t { MOUSE_MOVE, WHEEL_MOVE, MOUSE_DRAG } motion_type;
            std::optional<mouse_button> button = std::nullopt;

            ve_eq_comparable(key_t);
            ve_hashable();
        } input;

        keymods required_mods = KMOD_NONE;
        keymods excluded_mods = KMOD_NONE;

        enum trigger_when_t : u32 { MOTION_START = 0b001, MOTION_END = 0b010, MOTION_TICK = 0b100 };
        u32 trigger_when = MOTION_TICK;
        bool requires_mouse_capture = true;


        bool should_trigger(keymods mods, trigger_when_t when, std::optional<mouse_button> = std::nullopt) const {
            return (
                (trigger_when  & when) &&
                (input_manager::instance().has_mouse_capture() || !requires_mouse_capture) &&
                (required_mods & mods) == required_mods &&
                (excluded_mods & mods) == 0
            );
        }


        // Note: some state fields may contain the same value depending on what information is provided by the event.
        // E.g. for MOTION_START events all mouse_state fields are the same, since there was no last tick and the current state is the start state.
        struct handler_args {
            cref_of<mouse_state> begin, prev, current;
            keymods mods;
            trigger_when_t when;
            gfx::window* window;
        };

        using handler_t = std::function<void(const handler_args&)>;
    };


    // Maps inputs to actions. Inputs are obtained from the input manager.
    // I.e. provides a wrapper around the input manager that provides an interface more suitable for binding controls to actions.
    class input_binder {
    public:
        struct input_handle {
            std::variant<binary_input::key_t, motion_input::key_t> key;
            u32 id;
        };


        input_binder(void);
        ~input_binder(void);
        ve_immovable(input_binder);


        input_handle bind(binary_input input, typename binary_input::handler_t handler);
        input_handle bind(motion_input input, typename motion_input::handler_t handler);
        input_handle bind(std::string_view alias, typename binary_input::handler_t handler);
        input_handle bind(std::string_view alias, typename motion_input::handler_t handler);
        void unbind(input_handle handle);

        void alias(std::string name, binary_input input);
        void alias(std::string name, motion_input input);
        void unalias(std::string_view name);
    private:
        using event_handler_data = meta::pack<
            //         Event                         Motion Type               Trigger When                             Mouse Motion Type
            meta::pack<key_down_event,               binary_input, meta::value<binary_input::KEY_DOWN    >, meta::value<meta::null_type { }>>,
            meta::pack<key_up_event,                 binary_input, meta::value<binary_input::KEY_UP      >, meta::value<meta::null_type { }>>,
            meta::pack<key_hold_event,               binary_input, meta::value<binary_input::KEY_HOLD    >, meta::value<meta::null_type { }>>,
            meta::pack<button_press_event,           binary_input, meta::value<binary_input::KEY_DOWN    >, meta::value<meta::null_type { }>>,
            meta::pack<button_release_event,         binary_input, meta::value<binary_input::KEY_UP      >, meta::value<meta::null_type { }>>,
            meta::pack<button_hold_event,            binary_input, meta::value<binary_input::KEY_HOLD    >, meta::value<meta::null_type { }>>,
            meta::pack<mouse_move_start_event,       motion_input, meta::value<motion_input::MOTION_START>, meta::value<motion_input::key_t::MOUSE_MOVE>>,
            meta::pack<mouse_move_end_event,         motion_input, meta::value<motion_input::MOTION_END  >, meta::value<motion_input::key_t::MOUSE_MOVE>>,
            meta::pack<mouse_moved_event,            motion_input, meta::value<motion_input::MOTION_TICK >, meta::value<motion_input::key_t::MOUSE_MOVE>>,
            meta::pack<mouse_wheel_move_start_event, motion_input, meta::value<motion_input::MOTION_START>, meta::value<motion_input::key_t::WHEEL_MOVE>>,
            meta::pack<mouse_wheel_move_end_event,   motion_input, meta::value<motion_input::MOTION_END  >, meta::value<motion_input::key_t::WHEEL_MOVE>>,
            meta::pack<mouse_wheel_moved_event,      motion_input, meta::value<motion_input::MOTION_TICK >, meta::value<motion_input::key_t::WHEEL_MOVE>>,
            meta::pack<mouse_drag_start_event,       motion_input, meta::value<motion_input::MOTION_START>, meta::value<motion_input::key_t::MOUSE_DRAG>>,
            meta::pack<mouse_drag_end_event,         motion_input, meta::value<motion_input::MOTION_END  >, meta::value<motion_input::key_t::MOUSE_DRAG>>,
            meta::pack<mouse_drag_event,             motion_input, meta::value<motion_input::MOTION_TICK >, meta::value<motion_input::key_t::MOUSE_DRAG>>
        >;

        using event_types = typename event_handler_data::template expand_inside<meta::pack_ops::keys>;


        // Handles for this object's handlers within the input manager.
        template <typename Event> struct event_handler_handle {
            using type = Event;
            typename input_manager::handler_id id;
        };

        typename event_types
            ::template expand_outside<event_handler_handle>
            ::template expand_inside<std::tuple>
        handler_ids;


        hash_map<std::string, binary_input> binary_aliases;
        hash_map<std::string, motion_input> motion_aliases;


        // Storage for input event handlers.
        template <typename Input> struct handler_data { u32 id; Input input; typename Input::handler_t handler; };
        u32 next_id = 0;

        hash_map<typename binary_input::key_t, std::vector<handler_data<binary_input>>> binary_handlers;
        hash_map<typename motion_input::key_t, std::vector<handler_data<motion_input>>> motion_handlers;


        template <typename T> auto& handlers_for_t(void) {
            return meta::pick<std::is_same_v<T, binary_input>>(binary_handlers, motion_handlers);
        }


        void register_handlers(void);
        void unregister_handlers(void);
    };
}