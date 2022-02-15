#pragma once

#include <VoxelEngine/event/event_system.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/performance_timer.hpp>

using namespace ve::defs;


// List of different event dispatcher types used for testing.
using dispatcher_types = ve::meta::pack<
    ve::simple_event_dispatcher<false, u16, false>,
    ve::simple_event_dispatcher<false, u16, true>,
    ve::simple_event_dispatcher<true,  u16, false>,
    ve::simple_event_dispatcher<true,  u16, true>,

    ve::delayed_event_dispatcher<false, u16, false>,
    ve::delayed_event_dispatcher<false, u16, true>,
    ve::delayed_event_dispatcher<true,  u16, false>,
    ve::delayed_event_dispatcher<true,  u16, true>
>;


template <std::size_t I> struct event {
    static inline std::size_t counter = 0;
    bool recursive = false;
};

template <std::size_t... Is>
constexpr auto event_types_for_seq(std::index_sequence<Is...>) {
    return ve::meta::pack<event<Is>...>{};
}

template <std::size_t N> using gen_event_types = decltype(event_types_for_seq(std::make_index_sequence<N>()));


auto do_singlethreaded = [](auto pred, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) pred();
};


auto do_multithreaded = [](auto pred, std::size_t count, std::size_t num_threads = 8) {
    std::vector<std::thread> threads;
    std::size_t work = count;

    for (std::size_t i = 0; i < num_threads; ++i) {
        std::size_t thread_work_limit = (count + num_threads - 1) / num_threads;
        std::size_t thread_work = std::min(work, thread_work_limit);
        work -= thread_work;

        threads.emplace_back([&, thread_work] {
            for (std::size_t j = 0; j < thread_work; ++j) pred();
        });
    }

    for (auto& thread : threads) thread.join();
};


template <typename Events, typename Dispatcher, typename Invoker, typename Handler>
inline std::vector<typename Dispatcher::handler_id> add_handlers(Dispatcher& d, Handler& handler, std::size_t count, Invoker& invoker, bool profile = false) {
    std::vector<typename Dispatcher::handler_id> result;
    std::mutex mtx;

    auto impl = [&] {
        Events::foreach([&] <typename Event> {
            invoker(
                [&] {
                    auto id = d.template add_handler<Event>(handler);

                    {
                        std::unique_lock lock { mtx };
                        result.push_back(id);
                    }
                },
                count
            );
        });
    };

    if (profile) {
        ve::performance_timer timer { ve::cat("adding ", count * Events::size, " handlers") };
        impl();
    } else {
        impl();
    }

    return result;
}


template <typename Events, typename Dispatcher, typename Invoker>
inline void dispatch_events(Dispatcher& d, std::size_t count, Invoker& invoker, bool profile = false) {
    auto impl = [&] {
        Events::foreach([&] <typename Event> {
            invoker(
                [&] {
                    if constexpr (requires { typename Dispatcher::simple_event_dispatcher_tag; }) {
                        d.dispatch_event(Event {});
                    } else {
                        d.add_event(Event {});
                    }
                },
                count
            );
        });

        if constexpr (requires { typename Dispatcher::delayed_event_dispatcher_tag; }) {
            d.dispatch_events();
        }
    };

    if (profile) {
        ve::performance_timer timer { ve::cat("dispatching ", count * Events::size, " events") };
        impl();
    } else {
        impl();
    }
}


template <typename Events, typename Dispatcher, typename Invoker>
inline void remove_handlers(Dispatcher& d, const std::vector<typename Dispatcher::handler_id>& handlers, Invoker& invoker, bool profile = false) {
    std::atomic_uint32_t i = 0;

    auto impl = [&] {
        Events::foreach([&] <typename Event> () mutable {
            invoker(
                [&] { d.template remove_handler<Event>(handlers[i++]); },
                handlers.size() / Events::size
            );
        });
    };

    if (profile) {
        ve::performance_timer timer { ve::cat("removing ", handlers.size(), " handlers") };
        impl();
    } else {
        impl();
    }
}


template <typename Events, typename Dispatcher> inline test_result check_counters(std::size_t expected_count) {
    test_result result = VE_TEST_SUCCESS;

    Events::foreach([&] <typename Event> {
        if (Event::counter != expected_count) {
            result = VE_TEST_FAIL(
                "Incorrect number of calls to handle ", ctti::nameof<Event>(), " with ", ctti::nameof<Dispatcher>(), ": ",
                "expected ", expected_count, ", got ", Event::counter, "."
            );
        }

        Event::counter = 0;
        return (result == VE_TEST_SUCCESS);
    });

    return result;
}