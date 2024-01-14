#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/system_traits.hpp>
#include <VoxelEngine/ecs/system/scheduling/access_mode.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_invoke_wrapper.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_scheduler_data.hpp>

#include <thread>
#include <mutex>
#include <vector>


namespace ve::ecs::schedule {
    /**
     * The system scheduler stores systems and schedules them according to their traits to run on multiple threads.
     *
     * @internal The scheduler performs two basic actions:
     *
     * Setup (Whenever a task is added or removed):
     *  - Generate the list of dependencies for each task from the system_traits run_before/run_after/blacklist tags
     *  - Check for circular dependencies (debug mode only)
     *  - Generate the system's priority from its runtime estimate + the priorities of all its dependencies
     *  - Generate the starting-task set of all tasks without dependencies
     *
     * Invocation:
     *  - Use the set of starting-tasks as the set of available-tasks
     *  - While there are tasks in the available-tasks list:
     *        - Get the available-task with the highest priority that:
     *              - Can run on the current thread
     *              - Doesn't cause conflicts with the current component/entity read/write state
     *              - Isn't blacklisted through the dont-run-during tags of some other system
     *        - Mark any tasks that cannot run at the same time as blacklisted.
     *        - Invoke the task
     *        - Update the task performance counter
     *        - For each dependent of the current task:
     *              - Remove this task from its list of pending dependencies
     *              - If there are no more pending dependencies for the dependent, add it to the list of available-tasks
     *  - Update all task priorities with the new performance counters
     *
     *  TODO: The current setup is optimized for large systems with many dependencies, it might be better to define multiple scheduling strategies,
     *   and pick one based on the currently present systems.
     */
    class system_scheduler {
    public:
        system_scheduler(void);
        explicit system_scheduler(std::size_t num_worker_threads);
        ~system_scheduler(void);


        /**
         * Invokes all systems in the scheduler.
         * @param dt The amount of time simulated during the current tick.
         * @param now The timestamp of the current tick.
         */
        void invoke(time::duration dt, time::tick_timestamp now);


        /**
         * Adds a system to the scheduler and assigns it an unique ID.
         * @param system The system to add to the scheduler.
         * @param registry The registry this scheduler is part of.
         * @return The ID of the newly added system.
         */
        template <typename Registry, typename System> system_id add_system(System system, Registry* registry);


        /**
         * Removes a system from the scheduler.
         * @param system The ID of the system to remove.
         * @return True if the system was removed from the scheduler, or false if no system with the given ID existed.
         */
        bool remove_system(system_id system);


        /**
         * Removes the system from the scheduler and returns it as an std::optional.
         * If a system with the given ID exists, but it is of the wrong type, it is not removed.
         * @tparam System The type of the system to remove.
         * @param system The ID of the system to remove.
         * @return An std::optional containing the system if a system with the given type and ID existed, or std::nullopt otherwise.
         */
        template <typename System> std::optional<System> take_system(system_id system);


        /**
         * Returns a pointer to the system with the given ID, if a system with the given ID and type exists.
         * @tparam System The type of the system.
         * @param system The ID of the system.
         * @return A pointer to the system if it exists, or nullptr.
         */
        template <typename System> [[nodiscard]] System* get_system(system_id system);


        /** @copydoc get_system */
        template <typename System> [[nodiscard]] const System* get_system(system_id system) const;


        /** Returns true if a system with the given ID exists. */
        [[nodiscard]] bool has_system(system_id system) const;
    private:
        enum class invoke_result { SUCCESS, ALL_TASKS_DONE, NO_TASK_AVAILABLE };


        /** Per-system information, see @ref system_data. */
        hash_map<system_id, system_data> systems;
        /** Mapping from each tag to a list of all systems that have it in their traits' sequencing_tags. */
        hash_map<seq_tag_id, std::vector<system_id>> system_tags;
        /** Set to true before a tick if the set of systems in the scheduler changed. */
        bool systems_changed = false;
        /** Counter to assign IDs to systems. */
        system_id next_id = 0;

        /** Map of components to how they are accessed during system invocation. */
        hash_map<component_id, access_counter> component_access;
        /** Keeps track of how the entities are accessed during system invocation. */
        access_counter entity_access;
        /** Set of tasks that have all their dependencies fulfilled and can be executed now, sorted by priority. May still contain blacklisted systems. */
        tree_set<system_data*, priority_comparator> available_tasks;

        /** Worker threads to run systems on. */
        std::vector<std::thread> threads;
        /** Signal from worker threads that they have no more tasks to run. */
        std::condition_variable signal_thread_idle;
        /** Signal from main thread that there are new tasks to run. */
        std::condition_variable signal_tasks_ready;
        /** Mutex for access to system data, component and entity access maps, and the flags defined below. */
        std::mutex mutex;
        /** Counters for the number of tasks that have been completed this tick and that are running currently. */
        atom<u32> tasks_completed = 0, tasks_running = 0;
        /** Number of threads still working (including idle ones that may still run more tasks later this tick). A value of zero signals all tasks are done. */
        u32 busy_threads            = 0;
        /** Signals worker threads that there are tasks ready to be done. Used in conjunction with signal_tasks_ready. */
        bool tasks_ready            = false;
        /** Signals worker threads to exit. Used in conjunction with signal_tasks_ready. */
        bool join_workers           = false;
        /** Signals a system has exclusive access to the ECS. All other systems must wait. */
        atom<bool> exclusive_access = false;

        /** The amount of time simulated during the current tick. */
        time::duration tick_dt;
        /** Timestamp of the current tick. */
        time::tick_timestamp tick_timestamp;




        /** Assert that there are no systems which directly or indirectly have themselves as a dependency. */
        void assert_no_circular_dependencies(void);
        /** Assert that the scheduler is in a valid state before a new tick begins. */
        void assert_pre_tick_state(void);


        /**
         * Updates the set of dependencies, dependents and blacklist for the given system.
         * @tparam System The type of the system.
         * @param self The stored data for the system.
         */
        template <typename System> void rebuild_dependencies_for(system_data& self);


        /** Rebuilds the system data map after the set of stored systems changes. */
        void rebuild_data(void);
        /** Resets the state of each system. Called before each tick. */
        void reset_data(void);


        /**
         * Returns true if the given task can safely be executed now.
         * I.e. there are no components/entities it reads that are currently being written to, or vice versa,
         * there are no blacklisted systems already running currently, and its requirements regarding exclusivity and main-thread usage are satisfied.
         * @pre calling thread should hold the mutex lock.
         */
        bool can_invoke_now(const system_data* data) const;


        /**
         * Updates component/entity access counters, blacklist counters and exclusive access indicator for the given system.
         * Should be called before invoking the given system.
         * @param data The system that is about to be invoked.
         * @pre calling thread should hold the mutex lock.
         */
        void mark_task_invoked(system_data* data);


        /**
         * Undoes the changes made by @ref mark_task_invoked.
         * Should be called after invoking a given system.
         * @param data The system that was invoked.
         * @pre calling thread should <b>NOT</b> hold the mutex lock.
         */
        void mark_task_not_invoked(system_data* data);


        /**
         * Update dependents of this system to indicate one of its dependencies has been completed.
         * If there are no more dependencies for a dependent, it will be added to the available_tasks set.
         * @param data The system that was invoked.
         * @pre calling thread should <b>NOT</b> hold the mutex lock.
         */
        void mark_dependents_available(const system_data* data);


        /** Invokes all systems in the scheduler. */
        void invoke_all(void);


        /** Main method for worker threads. */
        void thread_main(void);


        /**
         * Runs the next available task, if there is one.
         *
         * @return One of the following:
         *  - SUCCESS if a task was run
         *  - NO_TASK_AVAILABLE if none of the remaining tasks can be run currently.
         *  - ALL_TASKS_DONE if there are no more tasks to run.
         */
        invoke_result run_next_task(void);
    };
}


#include <VoxelEngine/ecs/system/scheduling/system_scheduler.inl>