#include <VoxelEngine/input/input_binder.hpp>
#include <VoxelEngine/utility/tuple_foreach.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>


namespace ve {
    input_binder::input_binder(void) {
        register_handlers();
    }


    input_binder::~input_binder(void) {
        unregister_handlers();
    }


    input_binder::input_handle input_binder::bind(binary_input&& input, typename binary_input::handler_t&& handler) {
        auto key = input.input;

        binary_handlers[key].push_back(handler_data<binary_input> {
            .id      = next_id,
            .input   = std::move(input),
            .handler = std::move(handler)
        });

        return input_handle { key, next_id++ };
    }


    input_binder::input_handle input_binder::bind(motion_input&& input, typename motion_input::handler_t&& handler) {
        auto key = input.input;

        motion_handlers[key].push_back(handler_data<motion_input> {
            .id      = next_id,
            .input   = std::move(input),
            .handler = std::move(handler)
        });

        return input_handle { key, next_id++ };
    }


    void input_binder::unbind(input_handle handle) {
        std::visit(
            visitor {
                [&](const binary_input::key_t& key) { std::erase_if(binary_handlers[key], [&](const auto& data) { return data.id == handle.id; }); },
                [&](const motion_input::key_t& key) { std::erase_if(motion_handlers[key], [&](const auto& data) { return data.id == handle.id; }); }
            },
            handle.key
        );
    }


    // Returns the field within the event structure that represents the state at the start of whatever is being measured.
    template <typename Event> constexpr decltype(auto) event_begin_field(const Event& e) {
        if constexpr      (requires { e.begin_state; }) return e.begin_state;
        else if constexpr (requires { e.old_state;   }) return e.old_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a supported API for input binder.");
    }


    // Returns the field within the event structure that represents the state during the previous tick.
    template <typename Event> constexpr decltype(auto) event_prev_field(const Event& e) {
        if constexpr      (requires { e.old_state;   }) return e.old_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else if constexpr (requires { e.begin_state; }) return e.begin_state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a supported API for input binder.");
    }


    // Returns the field within the event structure that represents the state during this tick.
    template <typename Event> constexpr decltype(auto) event_current_field(const Event& e) {
        if constexpr      (requires { e.new_state;   }) return e.new_state;
        else if constexpr (requires { e.end_state;   }) return e.end_state;
        else if constexpr (requires { e.state;       }) return e.state;
        else if constexpr (requires { e.begin_state; }) return e.begin_state;
        else static_assert(meta::always_false_v<Event>, "Event does not have a supported API for input binder.");
    }


    // Provides the key in the binary_handlers map that holds handlers for the given event.
    template <typename Event, typename State> constexpr binary_input::key_t binary_event_key(const Event& e, const State& s) {
        if constexpr (requires { s.key; }) return s.key;
        else return s.button;
    }


    // Provides the key in the motion_handlers map that holds handlers for the given event.
    template <auto Category, typename Event, typename State> constexpr motion_input::key_t motion_event_key(const Event& e, const State& s) {
        if constexpr (requires { e.button; }) return { Category, e.button };
        else return { Category, std::nullopt };
    }


    void input_binder::register_handlers(void) {
        // Add event handlers for each set of (event type, input type, invoke when) in event handler data.
        event_handler_data::foreach([&] <typename Data> {
            [&] <typename Event, typename Input, auto When, auto Category> (meta::pack<Event, Input, meta::value<When>, meta::value<Category>>) {
                constexpr bool is_binary = std::is_same_v<Input, binary_input>;


                // Add the actual event handler.
                auto id = input_manager::instance().template add_handler<Event>([&](const Event& e) {
                    const auto& state = event_current_field(e);

                    const auto& map_key = meta::pick<is_binary>(
                        ve_wrap_callable(binary_event_key),
                        ve_wrap_callable(motion_event_key<Category>)
                    )(e, state);

                    auto it = handlers_for_t<Input>().find(map_key);
                    if (it == handlers_for_t<Input>().end()) return;


                    for (const auto& [id, input, handler] : it->second) {
                        // Check if the given handler should be invoked for the given input.
                        // The exact signature to call depends on the event type.
                        bool matches = meta::pick<(!is_binary && requires { e.button; })>(
                            [&](const auto& i, const auto& s, const auto& e) { return i.matches(s.mods, When, e.button); },
                            [&](const auto& i, const auto& s, const auto& e) { return i.matches(s.mods, When); }
                        )(input, state, e);


                        if (matches) {
                            if constexpr (is_binary) std::invoke(handler, state.mods, When);
                            else std::invoke(handler, event_begin_field(e), event_prev_field(e), event_current_field(e), state.mods, When);
                        }
                    }
                });


                std::get<event_handler_handle<Event>>(handler_ids) = { id };
            }(Data{});
        });
    }


    void input_binder::unregister_handlers(void) {
        tuple_foreach(handler_ids, [] <typename H> (const H& handler) {
            input_manager::instance().template remove_handler<typename H::type>(handler.id);
        });
    }
}