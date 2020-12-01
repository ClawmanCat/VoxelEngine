#include <VoxelEngine/tests/define_game_symbols.hpp>
#include <VoxelEngine/utils/priority.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/event/event_handler.hpp>
#include <VoxelEngine/event/delayed_prioritized_event_dispatcher.hpp>
#include <VoxelEngine/event/immediate_prioritized_event_dispatcher.hpp>

#include <tuple>

using namespace ve;
using namespace ve::events;


struct EventA : public event { int a = 5; };
struct EventB : public event { };
struct EventC : public event { };


template <bool threadsafe, bool use_direct_dispatcher>
int test_single_threaded(void) {
    using dispatcher_t = std::conditional_t<
        use_direct_dispatcher,
        immediate_prioritized_event_dispatcher<threadsafe>,
        delayed_prioritized_event_dispatcher<threadsafe, threadsafe>
    >;
    
    dispatcher_t dispatcher;
    
    
    bool error_occurred = false;
    bool high_priority_event_done = false;
    u64 event_a_sum = 0, event_c_sum = 0;
    
    
    event_handler handler_a = [&](const event& e) {
        auto& a_event = static_cast<const EventA&>(e);
        event_a_sum += a_event.a;
    };
    
    event_handler handler_b_high_priority = [&](const event& e) {
        if (high_priority_event_done) error_occurred = true;
        high_priority_event_done = true;
    };
    
    event_handler handler_b_low_priority = [&](const event& e) {
      if (!high_priority_event_done) error_occurred = true;
      high_priority_event_done = false;
    };
    
    event_handler handler_c = [&](const event& e) {
        ++event_c_sum;
    };
    
    
    dispatcher.template add_handler<EventA>(std::move(handler_a), priority::NORMAL);
    dispatcher.template add_handler<EventB>(std::move(handler_b_high_priority), priority::HIGH);
    dispatcher.template add_handler<EventB>(std::move(handler_b_low_priority), priority::LOW);
    
    
    for (int i = 0; i < 5; ++i) {
        std::vector<u64> ids;
        for (int j = 0; j < 5; ++j) ids.push_back(dispatcher.template add_handler<EventC>(handler_c, priority::NORMAL));
        
        for (int j = 0; j < 5; ++j) dispatcher.dispatch_event(EventA());
        for (int j = 0; j < 5; ++j) dispatcher.dispatch_event(EventB());
        for (int j = 0; j < 5; ++j) dispatcher.dispatch_event(EventC());
        
        if constexpr (!use_direct_dispatcher) dispatcher.process_events();
        
        for (u64 id : ids) dispatcher.template remove_handler<EventC>(id);
    }
    
    // 5 iterations x 5 events x 5 increment per event.
    if (event_a_sum != (5 * 5 * 5)) error_occurred = true;
    // 5 iterations x 5 handlers x 5 events.
    if (event_c_sum != (5 * 5 * 5)) error_occurred = true;
    
    return error_occurred ? -1 : 0;
}


// TODO: Test multithreaded when thread pool is finished.
int test_multi_threaded(void) {
    delayed_prioritized_event_dispatcher dispatcher;
    
    return 0;
}


int main() {
    int result_code = 0;
    
    result_code |= test_single_threaded<true,  true>();
    result_code |= test_single_threaded<false, true>();
    result_code |= test_single_threaded<true,  false>();
    result_code |= test_single_threaded<false, false>();
    
    result_code |= test_multi_threaded();
    
    return result_code;
}