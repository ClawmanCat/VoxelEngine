#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/event_system_common.hpp>
#include <VoxelEngine/event/event_system.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/math.hpp>


constexpr std::size_t num_event_types       = 5;
constexpr std::size_t num_handlers_per_type = 10;
// Note: recursive insert has a complexity of O(2^n) with respect to this parameter, so keep it small!
constexpr std::size_t num_events_per_type   = 10;




template <typename Dispatcher> test_result test_recursive_dispatch(void) {
    using event_types = gen_event_types<num_event_types>;

    Dispatcher d;
    auto do_n_threaded = ve::meta::pick<Dispatcher::is_threadsafe>(do_multithreaded, do_singlethreaded);


    VE_LOG_INFO(ve::cat("Testing ", ctti::nameof<Dispatcher>()));

    auto handler = [&] <typename Event> (const Event& e) {
        if (!e.recursive) {
            if constexpr (requires { typename Dispatcher::simple_event_dispatcher_tag; }) {
                d.dispatch_event(Event { true });
            } else {
                d.add_event(Event { true });
            }
        } else {
            ++Event::counter;
        }

        if constexpr (Dispatcher::is_cancellable) return false;
    };

    auto handlers = add_handlers<event_types>(d, handler, num_handlers_per_type, do_n_threaded);
    dispatch_events<event_types>(d, num_events_per_type, do_n_threaded);
    remove_handlers<event_types>(d, handlers, do_n_threaded);

    return check_counters<event_types, Dispatcher>(num_handlers_per_type * num_handlers_per_type * num_events_per_type);
}




template <typename Dispatcher> struct recursive_insert_handler {
    Dispatcher* d;

    template <typename Event>
    std::conditional_t<Dispatcher::is_cancellable, bool, void>
    operator()(const Event& event) const {
        ++Event::counter;

        d->template add_handler<Event>(recursive_insert_handler { d });
        if constexpr (Dispatcher::is_cancellable) return false;
    }
};


template <typename Dispatcher> test_result test_recursive_insert(void) {
    using event_types = gen_event_types<num_event_types>;

    Dispatcher d;
    auto do_n_threaded = ve::meta::pick<Dispatcher::is_threadsafe>(do_multithreaded, do_singlethreaded);


    VE_LOG_INFO(ve::cat("Testing ", ctti::nameof<Dispatcher>()));

    auto handler = recursive_insert_handler<Dispatcher> { &d };
    add_handlers<event_types>(d, handler, num_handlers_per_type, do_n_threaded);
    dispatch_events<event_types>(d, num_events_per_type, do_n_threaded);

    // For each event (e), the number of handlers (h) doubles, so for the value of the counter (c):
    //    c = sum from k = 0 to (e - 1) (2^k) h_0   (e - 1, since handlers added by the last event aren't called.)
    // => c = (2^e - 1) h_0
    // => c = ((1 << e) - 1) h_0
    return check_counters<event_types, Dispatcher>(num_handlers_per_type * ((1 << num_events_per_type) - 1));
}




template <typename Dispatcher> struct recursive_erase_handler {
    Dispatcher* d;

    std::vector<typename Dispatcher::handler_id>* ids;
    std::size_t index;

    template <typename Event>
    std::conditional_t<Dispatcher::is_cancellable, bool, void>
    operator()(const Event& event) const {
        ++Event::counter;

        d->template remove_handler<Event>((*ids)[index]);
        if constexpr (Dispatcher::is_cancellable) return false;
    }
};


template <typename Dispatcher> test_result test_recursive_erase(void) {
    using event_types = gen_event_types<num_event_types>;

    Dispatcher d;
    auto do_n_threaded = ve::meta::pick<Dispatcher::is_threadsafe>(do_multithreaded, do_singlethreaded);


    VE_LOG_INFO(ve::cat("Testing ", ctti::nameof<Dispatcher>()));

    std::vector<typename Dispatcher::handler_id> handlers;

    event_types::foreach([&, i = 0ull] <typename Event> () mutable {
        do_singlethreaded(
            [&] {
                auto id = d.template add_handler<Event>(
                    recursive_erase_handler<Dispatcher> { &d, &handlers, i++ }
                );

                handlers.push_back(id);
            },
            num_handlers_per_type
        );
    });

    // All handlers should remove themselves after first event, so counter should remain constant after that.
    dispatch_events<event_types>(d, num_events_per_type, do_n_threaded);
    return check_counters<event_types, Dispatcher>(num_handlers_per_type);
}




test_result test_main(void) {
    test_result result = VE_TEST_SUCCESS;


    VE_LOG_INFO("Testing recursive event dispatching...");
    dispatcher_types::foreach([&] <typename Dispatcher> {
        result = test_recursive_dispatch<Dispatcher>();
        return (result == VE_TEST_SUCCESS);
    });

    if (result != VE_TEST_SUCCESS) return result;


    VE_LOG_INFO("Testing recursive handler insertion...");
    dispatcher_types::foreach([&] <typename Dispatcher> {
        result = test_recursive_insert<Dispatcher>();
        return (result == VE_TEST_SUCCESS);
    });

    if (result != VE_TEST_SUCCESS) return result;


    VE_LOG_INFO("Testing recursive handler erasure...");
    dispatcher_types::foreach([&] <typename Dispatcher> {
        result = test_recursive_erase<Dispatcher>();
        return (result == VE_TEST_SUCCESS);
    });

    return result;
}