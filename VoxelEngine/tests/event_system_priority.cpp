#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/event_system_common.hpp>
#include <VoxelEngine/event/event_system.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/priority.hpp>

#include <thread>

using namespace ve::defs;


constexpr std::size_t num_event_types           = 10;
constexpr std::size_t num_handlers_per_priority = 10;


template <typename Event> struct priority_check {
    static inline u16 last_priority = ve::max_value<u16>;
    static inline bool incorrect_order_detected = false;
};


template <typename Dispatcher> struct handler {
    u16 priority;

    template <typename Event>
    auto operator()(const Event& event) const {
        if (priority_check<Event>::last_priority < priority) {
            priority_check<Event>::incorrect_order_detected = true;
        }

        priority_check<Event>::last_priority = priority;

        if constexpr (Dispatcher::is_cancellable) return false;
    }
};


template <typename Dispatcher> test_result test_dispatcher(void) {
    using event_types = gen_event_types<num_event_types>;


    Dispatcher d;
    auto do_n_threaded = ve::meta::pick<Dispatcher::is_threadsafe>(do_multithreaded, do_singlethreaded);


    VE_LOG_INFO(ve::cat("Testing ", ctti::nameof<Dispatcher>()));

    event_types::foreach([&] <typename Event> {
        do_n_threaded(
            [&] {
                for (const auto& p : ve::priorities) {
                    d.template add_handler<Event>(handler<Dispatcher> { p }, p);
                }
            },
            num_handlers_per_priority
        );
    });

    dispatch_events<event_types>(d, 1, do_n_threaded);


    test_result result = VE_TEST_SUCCESS;

    event_types::foreach([&] <typename Event> {
        if (priority_check<Event>::incorrect_order_detected) {
            result = VE_TEST_FAIL(
                "Incorrect handler execution order of ", ctti::nameof<Event>(), " with ", ctti::nameof<Dispatcher>(), "."
            );
        }

        priority_check<Event>::last_priority = ve::max_value<u16>;
        priority_check<Event>::incorrect_order_detected = false;

        return (result == VE_TEST_SUCCESS);
    });

    return result;
}


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    dispatcher_types::foreach([&] <typename Dispatcher> {
        result = test_dispatcher<Dispatcher>();
        return (result == VE_TEST_SUCCESS);
    });

    return result;
}
