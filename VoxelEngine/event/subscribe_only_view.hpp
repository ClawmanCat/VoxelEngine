#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Inherit from this class to only expose the dispatcher methods to add and remove handlers,
    // and not those to add and dispatch events.
    template <typename Dispatcher> struct subscribe_only_view : protected Dispatcher {
        using Dispatcher::Dispatcher;
        using Dispatcher::add_handler;
        using Dispatcher::add_raw_handler;
        using Dispatcher::add_one_time_handler;
        using Dispatcher::add_one_time_raw_handler;
        using Dispatcher::remove_handler;


        using raw_handler   = typename Dispatcher::raw_handler;
        using handler_token = typename Dispatcher::handler_token;
        using priority_t    = typename Dispatcher::priority_t;
        using lock_t        = typename Dispatcher::lock_t;
        using wrapped_t     = Dispatcher;

        constexpr static bool is_cancellable = Dispatcher::is_cancellable;
        constexpr static bool is_threadsafe  = Dispatcher::is_cancellable;

        template <typename Event>
        using handler_t = typename Dispatcher::template handler_t<Event>;
    };
}