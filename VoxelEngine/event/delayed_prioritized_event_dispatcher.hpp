#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/raii.hpp>
#include <VoxelEngine/utils/priority.hpp>
#include <VoxelEngine/utils/meta/bind.hpp>
#include <VoxelEngine/utils/meta/remove_return_type.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>
#include <VoxelEngine/threading/dummy_mutex.hpp>
#include <VoxelEngine/threading/try_lock_while.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/event/event_handler.hpp>
#include <VoxelEngine/event/priority_storage.hpp>
#include <VoxelEngine/event/priority_mode.hpp>

#include <ctti/type_id.hpp>

#include <mutex>
#include <vector>
#include <type_traits>
#include <functional>

#include <iostream>


namespace ve::events {
    // An event handler that stores events until process_events is called and runs event handlers according to their priority.
    // process_events should not be called concurrently.
    // threadsafe_dispatch: safely allow dispatching events from multiple threads.
    //    (e.g. calling dispatch_event and process_events from different threads.)
    // threadsafe_handlers: safely allow adding event handlers from multiple threads.
    template <
        bool threadsafe_dispatch  = true,
        bool threadsafe_handlers  = true,
        priority_mode mode        = priority_mode::PER_EVENT,
        typename Priority         = priority,
        Priority default_priority = Priority::NORMAL
    > class delayed_prioritized_event_dispatcher {
    public:
        void process_events(void) {
            {
                std::scoped_lock lock { handler_mtx, dispatch_mtx };
                auto dispatch_bool = raii_bool(is_dispatching);
                
                for (auto& [type_id, events_for_t] : events) {
                    if (events_for_t.empty()) continue;
                    
                    auto handlers_for_event = handlers.find(type_id);
                    if (handlers_for_event == handlers.end()) continue;
                    
                    
                    if constexpr (mode == priority_mode::PER_EVENT) {
                        for (const auto& evnt : events_for_t) {
                            for (const auto& handler : handlers_for_event->second) handler(*evnt);
                        }
                    } else {
                        for (const auto& handler : handlers_for_event->second) {
                            for (const auto& evnt : events_for_t) handler(*evnt);
                        }
                    }
                    
                    
                    events_for_t.clear();
                }
            }
            
            if constexpr (is_partially_threadsafe) {
                std::lock_guard lock { task_queue_mtx };
                auto task_queue_bool = raii_bool(is_running_tasks);
                
                for (const auto& [task, id] : task_queue) task();
                task_queue.clear();
            }
        }
        
        
        template <event_class Event> void dispatch_event(const Event&& evnt) {
            const auto event_id = ctti::type_id<Event>();
            
            if (try_lock_while(dispatch_mtx, [&]() { return !is_dispatching; })) {
                std::lock_guard lock { dispatch_mtx, std::adopt_lock };
                events[event_id].push_back(std::make_unique<Event>(std::move(evnt)));
            } else {
                // Can't insert the event right now, so enqueue a task to do so later.
                if constexpr (is_partially_threadsafe) {
                    std::lock_guard lock { task_queue_mtx };
        
                    task_queue.push_back({
                        // std::function must be copyable, which wouldn't work if the wrapped function object is not,
                        // so temporarily release the event from its unique_ptr.
                        [this, evnt = std::move(evnt)]() mutable {
                            dispatch_event<Event>(std::move(evnt));
                        },
                        std::nullopt
                    });
                }
            }
        }
        
        
        template <event_class Event, universal<event_handler> Handler>
        u64 add_handler(Handler&& handler, Priority p = default_priority) {
            const auto event_id   = ctti::type_id<Event>();
            const auto handler_id = handler.get_id();
            
            if (try_lock_while(handler_mtx, [&]() { return !is_dispatching; })) {
                std::lock_guard lock { handler_mtx, std::adopt_lock };
                handlers[event_id].insert(std::forward<Handler>(handler), p);
            } else {
                // Can't insert the handler right now, so enqueue a task to do so later.
                if constexpr (is_partially_threadsafe) {
                    std::lock_guard lock { task_queue_mtx };
    
                    task_queue.push_back({
                        [this, handler = std::forward<Handler>(handler), p]() mutable {
                            add_handler<Event>(std::move(handler), p);
                        },
                        handler_id
                    });
                }
            }
            
            return handler_id;
        }
        
        
        template <event_class Event> void remove_handler(u64 id) {
            if (try_lock_while(handler_mtx, [&]() { return !is_dispatching; })) {
                std::lock_guard handler_lock { handler_mtx, std::adopt_lock };
    
                if (auto it = handlers.find(ctti::type_id<Event>()); it != handlers.end()) {
                    bool removed = it->second.erase_if([&](const auto& handler) { return handler.get_id() == id; });
                    if (removed) return;
                }
                
                // The task might still be in the task queue.
                // We might also currently be running a task, but in that case if the add_event task
                // is also in the queue it was added after the remove_task one, so we don't have to remove it.
                if constexpr (is_partially_threadsafe) {
                    if (try_lock_while(task_queue_mtx, [&]() { return !is_running_tasks; })) {
                        std::lock_guard task_queue_lock { task_queue_mtx, std::adopt_lock };
        
                        auto it = std::find_if(
                            task_queue.begin(), task_queue.end(),
                            [&](const auto& pair) {
                                const auto& [task, task_id] = pair;
                                return task_id.has_value() && task_id.value() == id;
                            }
                        );
                        
                        if (it != task_queue.end()) task_queue.erase(it);
                    }
                }
            } else {
                // Can't remove the handler right now, so enqueue a task to do so later.
                if constexpr (is_partially_threadsafe) {
                    std::lock_guard lock { task_queue_mtx };
    
                    task_queue.push_back({
                        [this, id]() { remove_handler<Event>(id); },
                        std::nullopt
                    });
                }
            }
        }
        
    private:
        constexpr static bool is_partially_threadsafe = threadsafe_dispatch || threadsafe_handlers;
        
        hash_map<ctti::type_index, std::vector<unique<const event>>> events;
        hash_map<ctti::type_index, priority_storage<event_handler, Priority>> handlers;
    
    
        // If this is a threadsafe container, we have to avoid deadlock when an event handler dispatches another event
        // or adds/removes a handler during process_events. We can enqueue these tasks to be run later.
        using task_queue_t = std::conditional_t<
            is_partially_threadsafe,
            std::vector<
                std::pair<
                    std::function<void(void)>,
                    std::optional<u64>
                >
            >,
            meta::null_type
        >;
        [[no_unique_address]] task_queue_t task_queue;
        
        
        // If the container does not need to be threadsafe, we can switch out the mutexes with dummy ones.
        using dispatch_mtx_t = std::conditional_t<threadsafe_dispatch, std::mutex, dummy_mtx>;
        [[no_unique_address]] dispatch_mtx_t dispatch_mtx;
        
        using handler_mtx_t = std::conditional_t<threadsafe_handlers, std::mutex, dummy_mtx>;
        [[no_unique_address]] handler_mtx_t handler_mtx;
        
        using task_queue_mtx_t = std::conditional_t<is_partially_threadsafe, std::mutex, dummy_mtx>;
        [[no_unique_address]] task_queue_mtx_t task_queue_mtx;
        
        
        // If the container does not need to be threadsafe, there is no point in indicating the dispatch atomically.
        using dispatch_bool_t = std::conditional_t<is_partially_threadsafe, std::atomic_bool, bool>;
        dispatch_bool_t is_dispatching = false, is_running_tasks = false;
    };
}