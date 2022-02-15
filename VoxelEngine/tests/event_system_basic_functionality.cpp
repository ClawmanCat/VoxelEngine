#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/event_system_common.hpp>
#include <VoxelEngine/event/event_system.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/performance_timer.hpp>
#include <VoxelEngine/utility/string.hpp>

#include <thread>

using namespace ve::defs;


constexpr std::size_t num_event_types       = 10;
constexpr std::size_t num_handlers_per_type = 100;
constexpr std::size_t num_events_per_type   = 1000;
constexpr std::size_t num_handlers_total    = num_event_types * num_handlers_per_type;
constexpr std::size_t num_events_total      = num_event_types * num_events_per_type;



template <typename Dispatcher> test_result test_dispatcher(void) {
    using event_types = gen_event_types<num_event_types>;


    Dispatcher d;
    auto do_n_threaded = ve::meta::pick<Dispatcher::is_threadsafe>(do_multithreaded, do_singlethreaded);


    VE_LOG_INFO(ve::cat("Testing ", ctti::nameof<Dispatcher>()));

    auto handler = [] <typename Event> (const Event& e) {
        ++Event::counter;
        if constexpr (Dispatcher::is_cancellable) return false;
    };

    auto handlers = add_handlers<event_types>(d, handler, num_handlers_per_type, do_n_threaded, true);
    dispatch_events<event_types>(d, num_events_per_type, do_n_threaded, true);
    remove_handlers<event_types>(d, handlers, do_n_threaded, true);

    VE_LOG_INFO("\n", false);

    return check_counters<event_types, Dispatcher>(num_handlers_per_type * num_events_per_type);
}


test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;

    dispatcher_types::foreach([&] <typename Dispatcher> {
        result = test_dispatcher<Dispatcher>();
        return (result == VE_TEST_SUCCESS);
    });

    return result;
}
