#include <VoxelEngine/ecs/system/scheduling/system_scheduler.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/utility/graph/graph_utils.hpp>
#include <VoxelEngine/utility/graph/find_cycles.hpp>
#include <VoxelEngine/utility/threading/thread_info.hpp>
#include <VoxelEngine/utility/services/logger.hpp>

#include <condition_variable>


namespace ve::ecs::schedule {
    system_scheduler::system_scheduler(void) :
        system_scheduler(std::thread::hardware_concurrency() - 1)
    {}




    system_scheduler::system_scheduler(std::size_t num_worker_threads) {
        for (std::size_t i = 0; i < num_worker_threads; ++i) {
            threads.emplace_back(&system_scheduler::thread_main, this);

            auto result = set_thread_name(threads.back().native_handle(), std::format("VE ECS Worker {}", i));
            if (!result.has_value()) get_service<engine_logger>().error("Failed to set worker thread name: {}", result.error());
        }
    }




    system_scheduler::~system_scheduler(void) {
        VE_DEBUG_ONLY(assert_main_thread());

        join_workers = true;
        signal_tasks_ready.notify_all();

        for (auto& worker : threads) worker.join();
    }




    /**
     * Invokes all systems in the scheduler.
     * @param dt The amount of time simulated during the current tick.
     * @param now The timestamp of the current tick.
     */
    void system_scheduler::invoke(time::duration dt, time::tick_timestamp now) {
        VE_DEBUG_ONLY(assert_main_thread());
        VE_DEBUG_ONLY(assert_pre_tick_state());

        tick_dt = dt;
        tick_timestamp = now;

        if (std::exchange(systems_changed, false)) rebuild_data();
        reset_data();

        invoke_all();
    }




    /**
     * Removes a system from the scheduler.
     * @param system The ID of the system to remove.
     * @return True if the system was removed from the scheduler, or false if no system with the given ID existed.
     */
    bool system_scheduler::remove_system(system_id system) {
        VE_DEBUG_ONLY(assert_main_thread());

        if (auto it = systems.find(system); it != systems.end()) {
            auto node = systems.extract(it);
            systems_changed = true;

            node.mapped().system->on_removed();

            return true;
        } else return false;
    }




    /** Returns true if a system with the given ID exists. */
    [[nodiscard]] bool system_scheduler::has_system(system_id system) const {
        return systems.contains(system);
    }




    /** Assert that there are no systems which directly or indirectly have themselves as a dependency. */
    void system_scheduler::assert_no_circular_dependencies(void) {
        auto g = graph::make_graph_adapter<const system_data>(
            [&] { return systems | views::values | views::addressof; },
            [] (const system_data* sd) { return sd->dependencies | graph::connected_vertices_to_edges(sd); }
        );

        auto cycles      = graph::find_cycles(g);
        auto system_name = [] (const system_data* sd) { return sd->system->get_name(); };


        VE_ASSERT(
            cycles.empty(),
            "Circular dependencies detected:\n{}",
            ve::cat_range_with(
                "\n",
                cycles | views::transform([&] (const auto& c) { return cat_range_with(", ", c | views::transform(system_name)); })
            )
        );
    }




    /** Assert that the scheduler is in a valid state before a new tick begins. */
    void system_scheduler::assert_pre_tick_state(void) {
        for (const auto& [id, data] : systems) {
            const std::string system_name = std::format("{} (ID {})", data.system->get_name(), data.id);

            VE_ASSERT(!data.priority_stale,             "System priority was not updated for {}.", system_name);
            VE_ASSERT(data.remaining_dependencies == 0, "Previous invocation finished with unfulfilled dependencies for {}.", system_name);
            VE_ASSERT(data.blacklist_count == 0,        "Previous invocation finished without resetting blacklist count for {}.", system_name);
        }

        VE_ASSERT(available_tasks.empty(),       "Previous invocation finished with uninvoked tasks.");
        VE_ASSERT(tasks_completed == 0,      "Task completion counter was not reset.");
        VE_ASSERT(tasks_running == 0,        "Task running counter was not reset.");
        VE_ASSERT(busy_threads == 0,         "Busy thread counter did not reach zero.");
        VE_ASSERT(tasks_ready == false,      "Tasks ready flag was set before invocation.");
        VE_ASSERT(join_workers == false,     "Join workers flag was set.");
        VE_ASSERT(exclusive_access == false, "Exclusive access flag was set outside invocation.");
    }




    /** Rebuilds the system data map after the set of stored systems changes. */
    void system_scheduler::rebuild_data(void) {
        for (auto& [id, data] : systems) data.rebuild_dependencies(this, data);
        VE_DEBUG_ONLY(assert_no_circular_dependencies());
    }




    /** Resets the state of each system. Called before each tick. */
    void system_scheduler::reset_data(void) {
        auto update_priority = [] (const auto& self, system_data& data) -> void {
            if (data.priority_stale) {
                data.priority = data.performance.count();

                for (auto* dependent : data.dependents) {
                    self(self, *dependent);
                    data.priority += dependent->priority;
                }

                data.priority_stale = false;
            }
        };


        for (auto& [id, data] : systems) {
            data.remaining_dependencies = data.dependencies.size();
            update_priority(update_priority, data);
        }

        tasks_completed = 0;
    }




    /**
     * Returns true if the given task can safely be executed now.
     * I.e. there are no components/entities it reads that are currently being written to, or vice versa,
     * there are no blacklisted systems already running currently, and its requirements regarding exclusivity and main-thread usage are satisfied.
     * @pre calling thread should hold the mutex lock.
     */
    bool system_scheduler::can_invoke_now(const system_data* data) const {
        if (data->blacklist_count != 0) return false;
        if (data->require_main_thread && !thread_id::is_main_thread()) return false;
        if (data->require_exclusive && tasks_running != 0) return false;


        for (const auto& [component, mode] : data->component_access) {
            const auto& current_access = component_access.at(component);
            if (!current_access.can_add_with_mode(mode)) return false;
        }

        if (!entity_access.can_add_with_mode(data->entity_access)) return false;


        return true;
    }




    /**
     * Updates component/entity access counters, blacklist counters and exclusive access indicator for the given system.
     * Should be called before invoking the given system.
     * @param data The system that is about to be invoked.
     * @pre calling thread should hold the mutex lock.
     */
    void system_scheduler::mark_task_invoked(system_data* data) {
        for (auto* blacklisted : data->blacklist) ++(blacklisted->blacklist_count);
        if (data->require_exclusive) exclusive_access = true;


        for (const auto& [component, mode] : data->component_access) {
            auto& current_access = component_access.at(component);
            current_access.add_with_mode(mode);
        }

        entity_access.add_with_mode(data->entity_access);


        ++tasks_running;
    }




    /**
     * Undoes the changes made by @ref mark_task_invoked.
     * Should be called after invoking a given system.
     * @param data The system that was invoked.
     * @pre calling thread should <b>NOT</b> hold the mutex lock.
     */
    void system_scheduler::mark_task_not_invoked(system_data* data) {
        for (auto* blacklisted : data->blacklist) --(blacklisted->blacklist_count);
        exclusive_access = false;


        for (const auto& [component, mode] : data->component_access) {
            auto& current_access = component_access.at(component);
            current_access.remove_with_mode(mode);
        }

        entity_access.remove_with_mode(data->entity_access);


        --tasks_running;
    }




    /**
     * Update dependents of this system to indicate one of its dependencies has been completed.
     * If there are no more dependencies for a dependent, it will be added to the available_tasks set.
     * @param data The system that was invoked.
     * @pre calling thread should <b>NOT</b> hold the mutex lock.
     */
    void system_scheduler::mark_dependents_available(const system_data* data) {
        for (auto* dependent : data->dependents) {
            if (--dependent->remaining_dependencies == 0) {
                with_lock(mutex, [&] { available_tasks.insert(dependent); });
            }
        }
    }




    /** Invokes all systems in the scheduler. */
    void system_scheduler::invoke_all(void) {
        // Create the list of initial tasks.
        for (auto& [id, data] : systems) {
            if (data.dependencies.empty()) available_tasks.insert(&data);
        }


        // Notify threads we have tasks to run.
        with_lock(mutex, [&] {
            tasks_ready  = true;
            busy_threads = threads.size();
        });

        signal_tasks_ready.notify_all();


        // Run tasks on main thread as well.
        invoke_result task_invoke_result;

        do {
            task_invoke_result = run_next_task();
            if (task_invoke_result == invoke_result::NO_TASK_AVAILABLE) std::this_thread::yield();
        } while (task_invoke_result != invoke_result::ALL_TASKS_DONE);


        // Wait until all other threads are done, then return.
        if (with_lock(mutex, [&] { return busy_threads == 0; })) return;
        else wait(mutex, signal_thread_idle, [&] { return busy_threads == 0; });
    }




    /** Main method for worker threads. */
    void system_scheduler::thread_main(void) {
        wait(mutex, signal_tasks_ready, [&] { return tasks_ready || join_workers; });
        if (join_workers) [[unlikely]] return;


        invoke_result task_invoke_result;

        do {
            task_invoke_result = run_next_task();
            if (task_invoke_result == invoke_result::NO_TASK_AVAILABLE) std::this_thread::yield();
        } while (task_invoke_result != invoke_result::ALL_TASKS_DONE);


        with_lock(mutex, [&] { --busy_threads; });
        signal_thread_idle.notify_all();
    }




    /**
     * Runs the next available task, if there is one.
     *
     * @return One of the following:
     *  - SUCCESS if a task was run
     *  - NO_TASK_AVAILABLE if none of the remaining tasks can be run currently.
     *  - ALL_TASKS_DONE if there are no more tasks to run.
     *
     * @internal Concurrency Model:
     * 1. This function will lock the mutex and select a task from the available_tasks set when it is invoked.
     *    If a task is found, it will update the blacklist_count of all dependencies while still holding the lock.
     *    The component/entity access counters and exclusive-access indicator will also be updated.
     *    The rest of the function body is invoked without locking the mutex.
     * 2. The associated system is invoked.
     * 3. The performance measure of the system is updated. This is done without holding the lock
     *    as there are no other threads that access this variable during system invocation.
     * 4. The blacklist_count of the system and the remaining_dependencies counter of all its dependents are updated.
     *    This is done without holding the lock as these variables are atomic.
     * 5. If the remaining_dependencies counter of a dependent reaches zero the function will acquire the lock again
     *    and add it to the set of available_tasks.
     */
    system_scheduler::invoke_result system_scheduler::run_next_task(void) {
        // Select task if there is one.
        system_data* task = nullptr;


        invoke_result result = with_lock(mutex, [&] {
            if (tasks_completed == systems.size()) return invoke_result::ALL_TASKS_DONE;

            for (auto it = available_tasks.begin(); it != available_tasks.end(); ++it) {
                if (can_invoke_now(*it)) {
                    task = *it;
                    mark_task_invoked(*it);

                    available_tasks.erase(it);

                    return invoke_result::SUCCESS;
                }
            }

            return invoke_result::NO_TASK_AVAILABLE;
        });

        if (result != invoke_result::SUCCESS) return result;


        // Execute task.
        auto after_task = on_scope_exit([&] {
            // Mark task completed.
            mark_task_not_invoked(task);
            mark_dependents_available(task);

            ++tasks_completed;
        });

        time::duration dt = std::invoke(*(task->system), tick_dt, tick_timestamp);
        task->performance = dt;


        return result;
    }
}