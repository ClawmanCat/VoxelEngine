#pragma once

#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/event/event_handler.hpp>
#include <VoxelEngine/event/priority_storage.hpp>
#include <VoxelEngine/threading/dummy_mutex.hpp>
#include <VoxelEngine/threading/try_lock_while.hpp>
#include <VoxelEngine/threading/thread_id.hpp>
#include <VoxelEngine/threading/shared_lock_guard.hpp>
#include <VoxelEngine/utils/raii.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>

#include <ctti/type_id.hpp>

#include <type_traits>
#include <vector>
#include <optional>
#include <functional>
#include <utility>
#include <shared_mutex>
#include <mutex>


namespace ve::events {
    // An event handler that processes events directly as they come in and runs events according to their priority.
    template <
        bool threadsafe           = true,
        typename Priority         = priority,
        Priority default_priority = Priority::NORMAL
    > class immediate_prioritized_event_dispatcher {
    public:
        template <event_class Event> void dispatch_event(const Event& evnt) {
            {
                std::shared_lock lock { handler_mtx, std::defer_lock };

                auto ownership = conditional_raii {
                    [&]() { handlers_owning_thread = no_owning_thread; },
                    false
                };

                // Make sure not to lock if this thread already owns the mutex.
                if (handlers_owning_thread != thread_counter::get_thread_id()) {
                    lock.lock();

                    handlers_owning_thread = thread_counter::get_thread_id();
                    ownership.set_active(true);
                }

                if (auto it = handlers.find(ctti::type_id<Event>()); it != handlers.end()) {
                    for (const auto& handler : it->second) handler(evnt);
                }
            }
            
            run_tasks();
        }
    
    
        template <event_class Event, universal<event_handler> Handler>
        u64 add_handler(Handler&& handler, Priority p = default_priority) {
            auto handler_id = handler.get_id();
            
            // Make sure not to lock if this thread already owns the mutex.
            if (handlers_owning_thread != thread_counter::get_thread_id()) {
                std::lock_guard lock { handler_mtx };
        
                auto ownership = raii {
                    [&]() { handlers_owning_thread = thread_counter::get_thread_id(); },
                    [&]() { handlers_owning_thread = no_owning_thread; }
                };
    
                handlers[ctti::type_id<Event>()].insert(std::forward<Handler>(handler), p);
            } else {
                // Already locked somewhere down the call stack so we can't safely insert the handler right now.
                std::lock_guard lock { task_queue_mtx };
                
                task_queue.push_back({
                    [this, handler = std::forward<Handler>(handler), p]() mutable {
                        add_handler<Event>(std::move(handler), p);
                    },
                    handler_id
                });
            }
            
            return handler_id;
        }
    
    
        template <event_class Event> void remove_handler(u64 id) {
            // Make sure not to lock if this thread already owns the mutex.
            if (handlers_owning_thread != thread_counter::get_thread_id()) {
                std::lock_guard lock { handler_mtx };
        
                auto ownership = raii {
                    [&]() { handlers_owning_thread = thread_counter::get_thread_id(); },
                    [&]() { handlers_owning_thread = no_owning_thread; }
                };
    
                handlers[ctti::type_id<Event>()].erase_if([&](const auto& handler) { return handler.get_id() == id; });
            } else {
                // Already locked somewhere down the call stack so we can't safely remove the handler right now.
                std::lock_guard lock { task_queue_mtx };
                
                task_queue.push_back({
                    [this, id]() mutable {
                        remove_handler<Event>(id);
                    },
                    id
                });
            }
        }
    private:
        void run_tasks(void) {
            // Since running the tasks could cause events to be run recursively, which could cause access to the task queue,
            // we must move the tasks out of the queue before running them.
            std::unique_lock lock { task_queue_mtx };
            if (task_queue.empty()) return;
            
            auto temp_queue = std::move(task_queue);
            task_queue = task_queue_t();
            
            lock.unlock();
            
            
            for (const auto& [task, id] : temp_queue) task();
        }
        
        
        hash_map<ctti::type_index, priority_storage<event_handler, Priority>> handlers;
        
        
        // If an event tries to add or remove a handler, we can't do so directly since it would invalidate iterators
        // to the handler storage
        using task_queue_t = std::vector<
            std::pair<
                std::function<void(void)>,
                std::optional<u64>
            >
        >;
        [[no_unique_address]] task_queue_t task_queue;
        
        
        // Used to prevent recursive calls from the same thread.
        // (Calls from different threads are already handled by the mutex.)
        using handler_owner_id_t = std::conditional_t<threadsafe, std::atomic_uint64_t, u64>;
        
        constexpr static inline u64 no_owning_thread = thread_counter::null_thread;
        handler_owner_id_t handlers_owning_thread = no_owning_thread;
    
    
        // If the container does not need to be threadsafe, we can switch out the mutexes with dummy ones.
        using handler_mtx_t = std::conditional_t<threadsafe, std::shared_mutex, dummy_mtx>;
        [[no_unique_address]] handler_mtx_t handler_mtx;
        
        using task_queue_mtx_t = std::conditional_t<threadsafe, std::mutex, dummy_mtx>;
        [[no_unique_address]] task_queue_mtx_t task_queue_mtx;
    };
}