#include <VoxelEngine/tests/framework/framework.hpp>
#include <VoxelEngine/event/event_dispatcher.hpp>
#include <VoxelEngine/event/delayed_event_dispatcher.hpp>
#include <VoxelEngine/event/spatial_event_dispatcher.hpp>
#include <VoxelEngine/utility/random.hpp>

using namespace ve;
using namespace ve::tests;


struct test_event : public event {
    vec3f position;
};

constexpr u64 handler_count   = (u64) 1e5;
constexpr u64 iteration_count = (u64) 10;




// Assure adding and removing handlers to the dispatcher works, and that events are dispatched.
template <typename Dispatcher, bool GlobalEvents, bool GlobalHandlers>
i32 basic_functionality(void) {
    constexpr bool is_delayed_dispatcher = requires { typename Dispatcher::delayed_event_dispatcher_tag; };
    constexpr bool is_spatial_dispatcher = requires { typename Dispatcher::spatial_event_dispatcher_tag; };
    
    Dispatcher dispatcher;
    std::vector<event_handler_id> active_handlers;
    std::size_t call_count = 0, expected_call_count = 0;
    
    
    progress_indicator pc_create { .message = "Creating handlers", .task_count = handler_count };
    for (u32 i = 0; i < handler_count; ++i) {
        pc_create.update();
        
        auto handler = event_handler { [&](const event& e) { ++call_count; } };
        active_handlers.push_back(handler.get_id());
        
        if constexpr (GlobalHandlers) {
            dispatcher.template add_handler<test_event>(std::move(handler), engine_actor_id);
        } else {
            dispatcher.template add_handler<test_event>(
                std::move(handler),
                { 0, 0, 0 },
                10,
                engine_actor_id
            );
        }
    }
    
    
    // Each iteration, remove a random amount of handlers, then execute a task.
    // If the removal worked correctly, call_count should be incremented by the number of remaining handlers.
    progress_indicator pc_iterate { .message = "Iterating", .task_count = iteration_count };
    for (u32 i = 0; i < iteration_count; ++i) {
        pc_iterate.update();
        
        u32 remove_count = cheaprand::random_int<u32>(0, 1000);
        for (u32 j = 0; j < remove_count; j++) {
            // Move a random element to end of container.
            std::swap(
                *(active_handlers.begin() + cheaprand::random_int<u32>(0, u32(active_handlers.size()) - j)),
                *(active_handlers.begin() + active_handlers.size() - (j + 1))
            );
        }
        
        
        // Erase all swapped elements and remove them from the dispatcher.
        auto first_removed_element = active_handlers.begin() + (active_handlers.size() - (remove_count + 1));
        
        for (auto id : ranges::subrange(first_removed_element, active_handlers.end())) {
            if constexpr (is_spatial_dispatcher) {
                dispatcher.template remove_handler<test_event>(id, engine_actor_id);
            } else {
                dispatcher.template remove_handler<test_event>(id);
            }
        }
        
        active_handlers.erase(first_removed_element, active_handlers.end());
        
        
        // Run an event to make sure the handlers were removed correctly.
        if constexpr (is_delayed_dispatcher) {
            dispatcher.template dispatch_event<test_event>(test_event {}, engine_actor_id);
        } else if constexpr (GlobalEvents) {
            dispatcher.template dispatch_event<test_event>(test_event {});
        } else {
            dispatcher.template dispatch_event<test_event>(test_event {}, { 0, 0, 0 });
        }
        
        // If we're testing the delayed dispatcher, we still have to trigger the dispatch sequence.
        if constexpr (is_delayed_dispatcher) dispatcher.dispatch_all();
        
        expected_call_count += active_handlers.size();
    }
    
    
    if (call_count != expected_call_count) {
        VE_LOG_ERROR(std::to_string(difference(call_count, expected_call_count)) + " test cases failed.");
        return -1;
    } else {
        VE_LOG_INFO("Test completed successfully.");
        return 0;
    }
}

VE_TEST_WITH_TEMPLARGS(
    basic_functionality,
    PRE_INIT,
    // Test different dispatcher types
    ((event_dispatcher<false>, true, true))
    ((delayed_event_dispatcher<false>, true, true))
    // Test global and non-global events (spatial dispatcher)
    ((spatial_event_dispatcher<>, true, true))
    ((spatial_event_dispatcher<>, false, false))
    ((spatial_event_dispatcher<>, true, false))
    ((spatial_event_dispatcher<>, false, true))
);




// spatial_event_dispatcher: assure dispatcher correctly selects handlers based on range.
template <template <typename Pos> typename DistanceFn>
i32 handler_range(void) {
    spatial_event_dispatcher<DistanceFn> dispatcher;
    distance_functions::L1<vec3f> distance;
    std::size_t success_count = 0, failure_count = 0;
    
    
    progress_indicator pc_create { .message = "Checking handlers", .task_count = handler_count };
    
    for (std::size_t i = 0; i < handler_count; ++i) {
        pc_create.update();
        
        vec3f position {
            cheaprand::random_real(-1000.0f, 1000.0f),
            cheaprand::random_real(-1000.0f, 1000.0f),
            cheaprand::random_real(-1000.0f, 1000.0f)
        };
        
        float range = cheaprand::random_real(1.0f, 50.0f);
        
        
        std::size_t handler_call_count = 0;
        auto id = dispatcher.template add_handler<test_event>(
            event_handler { [&](const event& e) { ++handler_call_count; } },
            position,
            range,
            engine_actor_id
        );
        
        
        // 6 distances are within range, 5 are out of range.
        float distances[] = {
            0.0f, 0.001f, 0.999f, 0.1f * range, 0.5f * range, 0.999f * range,
            1.001f * range, 2.0f * range, 5.0f * range, 100.0f * range, 1000.0f * range
        };
        
        for (const auto& distance : distances) {
            vec3f direction = glm::normalize(vec3f {
                cheaprand::random_real(-1.0f, 1.0f),
                cheaprand::random_real(-1.0f, 1.0f),
                cheaprand::random_real(-1.0f, 1.0f)
            });
            
            dispatcher.template dispatch_event<test_event>(
                test_event { },
                position + direction * distance
            );
        }
        
        dispatcher.template remove_handler<test_event>(id, engine_actor_id);

        
        failure_count += difference(handler_call_count, 6ull);
        success_count += 11ull - difference(handler_call_count, 6ull);
    }


    if (failure_count) {
        VE_LOG_ERROR(std::to_string(failure_count) + " test cases failed.");
        return -1;
    } else {
        VE_LOG_INFO("Test completed successfully.");
        return 0;
    }
}


VE_TEST_WITH_TEMPLARGS(
    handler_range,
    PRE_INIT,
    ((distance_functions::L1))
    ((distance_functions::euclidean))
);