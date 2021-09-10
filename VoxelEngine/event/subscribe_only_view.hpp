#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Inherit from this class to only expose the dispatcher methods to add and remove handlers,
    // and not those to add and dispatch events.
    template <typename Dispatcher> struct subscribe_only_view : protected Dispatcher {
        using Dispatcher::Dispatcher;
        using Dispatcher::add_handler;
        using Dispatcher::add_one_time_handler;
        using Dispatcher::remove_handler;


        using handler_id = typename Dispatcher::handler_id;
        using priority_t = typename Dispatcher::priority_t;
        using lock_t     = typename Dispatcher::lock_t;

        constexpr static bool is_cancellable = Dispatcher::is_cancellable;
        constexpr static bool is_threadsafe  = Dispatcher::is_cancellable;

        template <typename Event>
        using handler_t = typename Dispatcher::template handler_t<Event>;
    };
}