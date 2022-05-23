#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event_handler_id.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>


namespace ve {
    template <bool Cancellable> struct default_handler_for {
        template <typename Event>
        using type = std::function<std::conditional_t<Cancellable, bool, void>(const Event&)>;
    };

    template <typename Event> using default_noncancellable_handler_t = typename default_handler_for<false>::template type<Event>;
    template <typename Event> using default_cancellable_handler_t    = typename default_handler_for<true>::template type<Event>;


    // Note: only addition and removal of handlers is handled in the base class,
    // since derived classes differ in whether they collect-then-dispatch-all events or simply dispatch them immediately,
    // and thus have different interfaces for adding events.
    // Deriving classes must override the following methods:
    //    template <typename Event> handler_token add_handler(Handler<Event> handler, Priority priority);
    //    template <typename Event> handler_token add_one_time_handler(Handler<Event> handler, Priority priority);
    //    template <typename Event> bool remove_handler(raw_handler id);
    //    template <typename Event> bool has_handlers_for(void) const;
    //    bool has_pending_events(void) const;
    template <typename Derived, template <typename Event> typename Handler, typename Priority> struct dispatcher {
        template <typename Event> using handler_t = Handler<Event>;
        using dispatcher_base_t = dispatcher<Derived, Handler, Priority>;
        using self_t            = Derived;
        using raw_handler       = event_handler_id_t;
        using handler_token     = event_handler_token;
        using priority_t        = Priority;


        // All event dispatchers are immovable objects, since RAII-tokens need to store a pointer to the dispatcher.
        dispatcher(void) = default;
        ve_immovable(dispatcher);


        // Adds a handler to be executed every time the dispatcher receives an event of the given type.
        template <typename Event>
        [[nodiscard]] handler_token add_handler(Handler<Event> handler, Priority priority = Priority(0)) {
            VE_CRTP_CHECK(Derived, template add_handler<Event>);
            return VE_CRTP_CALL(Derived, template add_handler<Event>, fwd(handler), priority);
        }


        // Equivalent to above, but returns a raw ID instead of a RAII-object.
        // The caller is responsible for removing the handler by calling remove_handler with the provided ID.
        template <typename Event>
        raw_handler add_raw_handler(Handler<Event> handler, Priority priority = Priority(0), ve_impl_dispatcher_default_arg) {
            ve_impl_dispatcher_plugin_raii_check;
            return add_handler(fwd(handler), priority).extract_id();
        }


        // Equivalent to the other overload of add_handler, but the event type is deduced from the function signature.
        template <
            typename Fn,
            typename Event = meta::nth_argument_base<Fn, 0>
        > [[nodiscard]] handler_token add_handler(Fn&& handler, Priority priority = Priority(0)) {
            return add_handler<Event>(fwd(handler), priority);
        }


        // Equivalent to above, but returns a raw ID instead of a RAII-object.
        // The caller is responsible for removing the handler by calling remove_handler with the provided ID.
        template <
            typename Fn,
            typename Event = meta::nth_argument_base<Fn, 0>
        > raw_handler add_raw_handler(Fn&& handler, Priority priority = Priority(0), ve_impl_dispatcher_default_arg) {
            ve_impl_dispatcher_plugin_raii_check;
            return add_handler<Fn, Event>(fwd(handler), priority).extract_id();
        }


        // Equivalent to add_handler, but the handler is only invoked the first time an event of the given type is received,
        // and is then automatically removed from the dispatcher.
        template <typename Event>
        [[nodiscard]] handler_token add_one_time_handler(Handler<Event> handler, Priority priority = Priority(0)) {
            VE_CRTP_CHECK(Derived, template add_one_time_handler<Event>);
            return VE_CRTP_CALL(Derived, template add_one_time_handler<Event>, fwd(handler), priority);
        }


        // Equivalent to above, but returns a raw ID instead of a RAII-object.
        // The caller is responsible for removing the handler by calling remove_handler with the provided ID.
        template <typename Event>
        raw_handler add_one_time_raw_handler(Handler<Event> handler, Priority priority = Priority(0), ve_impl_dispatcher_default_arg) {
            ve_impl_dispatcher_plugin_raii_check;
            return add_one_time_handler<Event>(fwd(handler), priority);
        }


        // Equivalent to the other overload of add_one_time_handler, but the event type is deduced from the function signature.
        template <
            typename Fn,
            typename Event = meta::nth_argument_base<Fn, 0>
        > [[nodiscard]] handler_token add_one_time_handler(Fn&& handler, Priority priority = Priority(0)) {
            return add_one_time_handler<Event>(fwd(handler), priority);
        }


        // Equivalent to above, but returns a raw ID instead of a RAII-object.
        // The caller is responsible for removing the handler by calling remove_handler with the provided ID.
        template <
            typename Fn,
            typename Event = meta::nth_argument_base<Fn, 0>
        > raw_handler add_one_time_raw_handler(Fn&& handler, Priority priority = Priority(0), ve_impl_dispatcher_default_arg) {
            ve_impl_dispatcher_plugin_raii_check;
            return add_one_time_handler<Fn, Event>(fwd(handler), priority).extract_id();
        }


        // Removes the handler with the given ID, if it exists.
        // You only need to call this manually if you are using the raw_handler methods.
        template <typename Event> void remove_handler(raw_handler id) {
            VE_CRTP_CHECK(Derived, template remove_handler<Event>);
            VE_CRTP_CALL(Derived, template remove_handler<Event>, id);
        }


        // Removes the handler associated with the given token, if the token is valid.
        // This can be used to easily remove handlers before the token is destroyed
        // (E.g. if it is stored as a class member).
        template <typename Event> void remove_handler(handler_token&& token) {
            handler_token other = std::move(token); // Removed from destructor.
        }


        // If this method returns false, the dispatcher guarantees there are no handlers for the given event type present.
        // If this method returns true, there may or may not be handlers for the given event type.
        // This method can be called as an optimization before dispatching large numbers of events of the same type.
        template <typename Event> bool has_handlers_for(void) const {
            VE_CRTP_CHECK(Derived, template has_handlers_for<Event>);
            return VE_CRTP_CALL(Derived, template has_handlers_for<Event>);
        }


        // Returns true if there are any events in the dispatcher that have not yet been dispatched.
        // For dispatchers that dispatch their events immediately, this method always returns false.
        bool has_pending_events(void) const {
            VE_CRTP_CHECK(Derived, has_pending_events);
            return VE_CRTP_CALL(Derived, has_pending_events);
        }
    };


    #define ve_impl_using_dispatcher_fns(base)  \
    using base::add_handler;                    \
    using base::add_one_time_handler;           \
    using base::remove_handler;
}