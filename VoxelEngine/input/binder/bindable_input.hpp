#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/keyboard.hpp>
#include <VoxelEngine/input/mouse.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/traits/const_as.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>


namespace ve {
    namespace detail {
        struct type_erased_handler {
            fn<void, const void*, const void*, std::size_t> wrapped;
            const void* data;

            void operator()(const auto& event) const {
                std::invoke(wrapped, &event, data, type_hash<std::remove_cvref_t<decltype(event)>>());
            }
        };
    }


    class input {
    public:
        virtual ~input(void) = default;

        // Note: one set of handlers is registered per derived class (as determined by type_hash),
        // which invokes should_trigger for every instance of that derived class.
        virtual std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler) const = 0;
        virtual void unregister_handlers(const std::vector<event_handler_id_t>&) const = 0;
        virtual bool should_trigger(const void* event, std::size_t type) const = 0;

        std::size_t type_hash;

    protected:
        explicit input(std::size_t type_hash) : type_hash(type_hash) {}


        template <typename T> static const T* safe_cast(const void* event, std::size_t hash) {
            if (hash == ve::type_hash<T>()) return (const T*) event;
            else return nullptr;
        }

        template <typename... Events> static std::vector<event_handler_id_t> register_types(auto handler) {
            return { input_manager::instance().template add_handler<Events>(handler)... };
        }

        template <typename... Events> static void unregister_types(const std::vector<event_handler_id_t>& handlers) {
            meta::pack<Events...>::foreach_indexed([&] <typename E, std::size_t I> {
                input_manager::instance().template remove_handler<E>(handlers[I]);
            });
        }
    };


    class binary_input : public input {
    public:
        enum trigger_on_t : u8 {
            KEY_DOWN  = nth_bit(0),
            KEY_UP    = nth_bit(1),
            KEY_HOLD  = nth_bit(2),
            KEY_TYPED = nth_bit(3)
        };

        std::underlying_type_t<trigger_on_t> trigger_on;


        keymods required_mods, excluded_mods;
        bool requires_mouse_capture;

    protected:
        binary_input(std::size_t type_hash, const auto& args) :
            input(type_hash),
            trigger_on(args.trigger_on),
            required_mods(args.required_mods),
            excluded_mods(args.excluded_mods),
            requires_mouse_capture(args.requires_mouse_capture)
        {}


        static bool check_base_fields(const binary_input* i, const auto* event) {
            if (!event) return false;
            const auto& state = get_most_recent_state(*event);

            if (i->requires_mouse_capture && !input_manager::instance().has_mouse_capture()) return false;
            if ((state.mods & i->required_mods) != i->required_mods) return false;
            if ((state.mods & i->excluded_mods) != KMOD_NONE) return false;

            return true;
        }
    };


    class motion_input : public input {
    public:
        enum trigger_on_t : u8 {
            MOTION_START = nth_bit(0),
            MOTION_TICK  = nth_bit(1),
            MOTION_END   = nth_bit(2)
        };

        std::underlying_type_t<trigger_on_t> trigger_on;


        keymods required_mods, excluded_mods;
        bool requires_mouse_capture;

    protected:
        motion_input(std::size_t type_hash, const auto& args) :
            input(type_hash),
            trigger_on(args.trigger_on),
            required_mods(args.required_mods),
            excluded_mods(args.excluded_mods),
            requires_mouse_capture(args.requires_mouse_capture)
        {}


        static bool check_base_fields(const motion_input* i, const auto* event) {
            if (!event) return false;
            const auto& state = get_most_recent_state(*event);

            if (i->requires_mouse_capture && !input_manager::instance().has_mouse_capture()) return false;
            if ((state.mods & i->required_mods) != i->required_mods) return false;
            if ((state.mods & i->excluded_mods) != KMOD_NONE) return false;

            return true;
        }
    };


    class keyboard_input : public binary_input {
    public:
        using event_types = meta::pack<key_down_event, key_up_event, key_hold_event, key_type_event>;

        struct args {
            SDL_Keycode key;
            decltype(binary_input::trigger_on) trigger_on = binary_input::KEY_HOLD;
            keymods required_mods = KMOD_NONE;
            keymods excluded_mods = KMOD_NONE;
            bool requires_mouse_capture = true;
        };


        explicit keyboard_input(SDL_Keycode key) : keyboard_input(args { .key = key }) {}
        explicit keyboard_input(const struct args& args) : binary_input(ve::type_hash<keyboard_input>(), args), key(args.key) {}


        std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler h) const override {
            return event_types::apply([&] <typename... Ts> { return register_types<Ts...>(h); });
        }

        void unregister_handlers(const std::vector<event_handler_id_t>& hs) const override {
            event_types::apply([&] <typename... Ts> { return unregister_types<Ts...>(hs); });
        }

        bool should_trigger(const void* event, std::size_t type) const override {
            bool result = false;

            event_types::foreach_indexed([&] <typename E, std::size_t I> {
                if (binary_input::trigger_on & (binary_input::trigger_on_t) 1 << I) {
                    const auto* e = safe_cast<E>(event, type);
                    result |= check_base_fields(this, e) && get_most_recent_state(*e).key == key;
                }
            });

            return result;
        }


        SDL_Keycode key;
    };


    class mouse_button_input : public binary_input {
    public:
        using event_types = meta::pack<button_press_event, button_release_event, button_hold_event, button_hold_event>;

        struct args {
            mouse_button button;
            decltype(binary_input::trigger_on) trigger_on = binary_input::KEY_HOLD;
            keymods required_mods = KMOD_NONE;
            keymods excluded_mods = KMOD_NONE;
            bool requires_mouse_capture = true;
        };


        explicit mouse_button_input(mouse_button button) : mouse_button_input(args { .button = button }) {}
        explicit mouse_button_input(const struct args& args) : binary_input(ve::type_hash<mouse_button_input>(), args), button(args.button) {}


        std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler h) const override {
            return event_types::apply([&] <typename... Ts> { return register_types<Ts...>(h); });
        }

        void unregister_handlers(const std::vector<event_handler_id_t>& hs) const override {
            event_types::apply([&] <typename... Ts> { return unregister_types<Ts...>(hs); });
        }

        bool should_trigger(const void* event, std::size_t type) const override {
            bool result = false;

            event_types::foreach_indexed([&] <typename E, std::size_t I> {
                if (binary_input::trigger_on & (binary_input::trigger_on_t) 1 << I) {
                    const auto* e = safe_cast<E>(event, type);
                    result |= check_base_fields(this, e) && get_most_recent_state(*e).button == button;
                }
            });

            return result;
        }


        mouse_button button;
    };


    class mouse_motion_input : public motion_input {
    public:
        using event_types = meta::pack<mouse_move_start_event, mouse_moved_event, mouse_move_end_event>;

        struct args {
            decltype(motion_input::trigger_on) trigger_on = motion_input::MOTION_TICK;
            keymods required_mods = KMOD_NONE;
            keymods excluded_mods = KMOD_NONE;
            bool requires_mouse_capture = true;
        };


        mouse_motion_input(void) : mouse_motion_input(args { }) {}
        explicit mouse_motion_input(const struct args& args) : motion_input(ve::type_hash<mouse_motion_input>(), args) {}


        std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler h) const override {
            return event_types::apply([&] <typename... Ts> { return register_types<Ts...>(h); });
        }

        void unregister_handlers(const std::vector<event_handler_id_t>& hs) const override {
            event_types::apply([&] <typename... Ts> { return unregister_types<Ts...>(hs); });
        }

        bool should_trigger(const void* event, std::size_t type) const override {
            bool result = false;

            event_types::foreach_indexed([&] <typename E, std::size_t I> {
                if (motion_input::trigger_on & (motion_input::trigger_on_t) 1 << I) {
                    const auto* e = safe_cast<E>(event, type);
                    result |= check_base_fields(this, e);
                }
            });

            return result;
        }
    };


    class mouse_wheel_motion_input : public motion_input {
    public:
        using event_types = meta::pack<mouse_wheel_move_start_event, mouse_wheel_moved_event, mouse_wheel_move_end_event>;

        struct args {
            decltype(motion_input::trigger_on) trigger_on = motion_input::MOTION_TICK;
            keymods required_mods = KMOD_NONE;
            keymods excluded_mods = KMOD_NONE;
            bool requires_mouse_capture = true;
        };


        mouse_wheel_motion_input(void) : mouse_wheel_motion_input(args { }) {}
        explicit mouse_wheel_motion_input(const struct args& args) : motion_input(ve::type_hash<mouse_wheel_motion_input>(), args) {}


        std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler h) const override {
            return event_types::apply([&] <typename... Ts> { return register_types<Ts...>(h); });
        }

        void unregister_handlers(const std::vector<event_handler_id_t>& hs) const override {
            event_types::apply([&] <typename... Ts> { return unregister_types<Ts...>(hs); });
        }

        bool should_trigger(const void* event, std::size_t type) const override {
            bool result = false;

            event_types::foreach_indexed([&] <typename E, std::size_t I> {
                if (motion_input::trigger_on & (motion_input::trigger_on_t) 1 << I) {
                    const auto* e = safe_cast<E>(event, type);
                    result |= check_base_fields(this, e);
                }
            });

            return result;
        }
    };


    class mouse_drag_input : public motion_input {
    public:
        using event_types = meta::pack<mouse_drag_start_event, mouse_drag_event, mouse_drag_end_event>;

        struct args {
            mouse_button button;
            decltype(motion_input::trigger_on) trigger_on = motion_input::MOTION_TICK;
            keymods required_mods = KMOD_NONE;
            keymods excluded_mods = KMOD_NONE;
            bool requires_mouse_capture = true;
        };


        explicit mouse_drag_input(mouse_button button) : mouse_drag_input(args { .button = button }) {}
        explicit mouse_drag_input(const struct args& args)  : motion_input(ve::type_hash<mouse_drag_input>(), args), button(args.button) {}


        std::vector<event_handler_id_t> register_handlers(detail::type_erased_handler h) const override {
            return event_types::apply([&] <typename... Ts> { return register_types<Ts...>(h); });
        }

        void unregister_handlers(const std::vector<event_handler_id_t>& hs) const override {
            event_types::apply([&] <typename... Ts> { return unregister_types<Ts...>(hs); });
        }

        bool should_trigger(const void* event, std::size_t type) const override {
            bool result = false;

            event_types::foreach_indexed([&] <typename E, std::size_t I> {
                if (motion_input::trigger_on & (motion_input::trigger_on_t) 1 << I) {
                    const auto* e = safe_cast<E>(event, type);
                    result |= check_base_fields(this, e) && e->button == button;
                }
            });

            return result;
        }


        mouse_button button;
    };
}