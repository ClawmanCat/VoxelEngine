#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/is_immovable.hpp>
#include <VoxelEngine/utility/traits/value.hpp>


namespace ve {
    using event_handler_id_t = u32;


    // RAII-token for event handlers.
    // Event handler is automatically removed from the event dispatcher when the token is destroyed.
    template <typename Dispatcher> class event_handler_token {
    public:
        event_handler_token(void) = default;
        event_handler_token(const event_handler_token&) = delete;
        event_handler_token& operator=(const event_handler_token&) = delete;


        template <typename Event> event_handler_token(meta::type_wrapper<Event>, typename Dispatcher::raw_handler id, Dispatcher* dispatcher)
            : data(data_t { id, dispatcher, make_remove_handler_fn<Event>() })
        {}


        event_handler_token(event_handler_token&& other) { *this = std::move(other); }

        event_handler_token& operator=(event_handler_token&& other) {
            destroy_token();
            std::swap(data, other.data);
        }


        ~event_handler_token(void) {
            destroy_token();
        }


        void destroy_token(void) {
            if (data) {
                std::invoke(data->call_remove_handler, data->dispatcher, data->id);
                data = std::nullopt;
            }
        };


        typename Dispatcher::raw_handler extract_id(void) {
            VE_DEBUG_ASSERT(data, "Cannot extract ID from empty handler RAII object.");
            return std::exchange(data, std::nullopt)->id;
        }

        bool has_value(void) const { return data.has_value(); }
    private:
        struct data_t {
            typename Dispatcher::raw_handler id;
            Dispatcher* dispatcher;
            fn<void, Dispatcher*, typename Dispatcher::raw_handler> call_remove_handler;
        };

        std::optional<data_t> data;


        void assert_immovable(void) {
            // Cannot do this as a requires clause since the dispatcher type will likely be incomplete when this template is specialized.
            static_assert(meta::is_immovable_v<Dispatcher>, "Cannot create RAII token for movable dispatcher type.");
        }


        template <typename Event> static auto make_remove_handler_fn(void) {
            return [] (Dispatcher* dispatcher, typename Dispatcher::raw_handler id) {
                dispatcher->template remove_handler<Event>(id);
            };
        }
    };


    // Plugins should typically not use non-RAII event handlers, since if the plugin is unloaded,
    // the callback that was given to the dispatcher will be unloaded as well.
    #if defined(VE_BUILD_PLUGIN) || defined(VE_PLUGIN_NO_EVENT_RAII_WARNING)
        #define ve_impl_dispatcher_default_arg ve::u32 ve_impl_default_arg = 1
    #else
        #define ve_impl_dispatcher_default_arg ve::u32 ve_impl_default_arg = 0
    #endif

    #define ve_impl_dispatcher_plugin_raii_check                                            \
        VE_DEBUG_ONLY(                                                                      \
            if (ve_impl_default_arg) {                                                      \
                VE_LOG_WARN(ve::cat(                                                        \
                    "Calling this method from a plugin is not recommended. ",               \
                    "If the plugin is unloaded, but the event handler remains, ",           \
                    "the game will crash the next time the handler is called.\n",           \
                    "You should use the RAII-version of this method instead, ",             \
                    "to ensure the handler is removed when the plugin is unloaded.\n",      \
                    "You can compile your plugin with VE_PLUGIN_NO_EVENT_RAII_WARNING ",    \
                    "to mute this warning."                                                 \
                ));                                                                         \
            }                                                                               \
        )
}