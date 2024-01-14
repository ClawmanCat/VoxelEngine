#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/threading/threading_utils.hpp>
#include <VoxelEngine/utility/threading/mutexed.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/worker_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/scheduler_info.hpp>

#include <variant>


namespace ve::ecs::schedule {
    /** Indicates that there are no tasks available for this thread currently (More might become available later during the tick). */
    struct no_tasks_available {};
    /** Indicates that there are no more tasks remaining that can be executed by this thread this tick. */
    struct no_tasks_remaining {};

    template <typename R> using task        = task_polymorphic_wrapper<R>;
    template <typename R> using maybe_task  = std::variant<task<R>*, no_tasks_available, no_tasks_remaining>;


    /**
     * Defines a strategy for scheduling tasks in the system_scheduler. Deriving classes should implement next_task() to provide the next task for a given thread.
     * @tparam R The type of the associated registry.
     */
    template <typename R> struct scheduling_strategy {
        virtual ~scheduling_strategy(void) = default;

        /** Invoked just after a task is added to the scheduler. */
        virtual void on_task_added(const scheduler_info<R>& scheduler, const task_info<R>& task) {}
        /** Invoked just before a task is removed from the scheduler. */
        virtual void on_task_removed(const scheduler_info<R>& scheduler, const task_info<R>& task) {}

        /** Invoked just before a new tick is started. */
        virtual void on_tick_start(const scheduler_info<R>& scheduler) {}
        /** Invoked just after a tick is completed. */
        virtual void on_tick_completed(const scheduler_info<R>& scheduler) {}

        /**
         * Invoked during a tick whenever a worker needs a new task.
         *
         * @returns
         *  - If there are any tasks that can be done, this method should return a pointer to the next task.
         *  - If there are no tasks that can currently be done but not all tasks have been completed, this method should return an instance of no_tasks_available.
         *  - If all tasks have been completed, this method should return an instance of no_tasks_remaining.
         * @warning This method is invoked concurrently by the scheduler and should be made thread-safe.
         * @note The strategy is allowed to return task objects that are not in scheduler_info::tasks, as long as all tasks in said set are executed before returning no_tasks_remaining.
         */
        VE_CALLED_CONCURRENTLY virtual maybe_task<R> try_start_task(const scheduler_info<R>& scheduler, const worker_info& worker) = 0;

        /**
         * Invoked during a tick whenever a worker completes a task.
         * @warning This method is invoked concurrently by the scheduler and should be made thread-safe.
         */
        VE_CALLED_CONCURRENTLY virtual void complete_task(const scheduler_info<R>& scheduler, const worker_info& worker, const task_info<R>& task) {}
    };
}