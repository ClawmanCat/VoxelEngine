#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/utility/threading/threading_utils.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/worker_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/access_tracker.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/scheduler_info.hpp>

#include <mutex>


namespace ve::ecs::schedule {
    /**
     * Strategy that maximizes the amount of available work by prioritizing tasks that lock a lot of execution behind themselves as dependents.
     * Optimized for situations where there are a few large systems and/or a large amount of internal dependencies.
     *
     * @internal The strategy works as follows:
     * - A set is kept of all currently available tasks (tasks with no unfulfilled dependencies), sorted by their priority
     *   (calculated as the runtime of the system + the priority of all its dependencies).
     *   Initially, this set is filled with all tasks with no dependencies.
     * - Whenever try_start_task is invoked, the highest-priority task from the set is returned that is currently available
     *   (not blacklisted by another task, allowed to run on the current thread and not blocked due to component-access, entity-access or exclusive-access rules).
     * - Whenever complete_task is invoked, all dependents of the task which have no other unfulfilled dependencies are added to the available task set.
     */
    template <typename R> class maximize_available_work : public scheduling_strategy<R> {
    public:
        void on_tick_start(const scheduler_info<R>& scheduler) override {
            VE_DEBUG_ASSERT(
                !ecs_access.has_ongoing_access(),
                "Resources still marked as in-use after completion of tick."
            );

            VE_DEBUG_ASSERT(
                available_tasks.empty(),
                "Not all tasks were completed after the last tick."
            );


            task_data.clear();

            for (const auto& [id, task] : scheduler.tasks) {
                if (!task_data.contains(&task)) create_task_data(task);
            }

            tasks_started = 0;
        }


        VE_CALLED_CONCURRENTLY maybe_task<R> try_start_task(const scheduler_info<R>& scheduler, const worker_info& worker) override {
            return with_lock(mutex, [&] {
                if (available_tasks.empty()) {
                    return (tasks_started == task_data.size())
                        ? no_tasks_remaining {}
                        : no_tasks_available {};
                }


                for (auto it = available_tasks.begin(); it != available_tasks.end(); ++it) {
                    const auto& data = *it;
                    const auto& task = *data.task;

                    if (data.blacklisted_by_count != 0) continue;
                    if (ecs_access.has_access_conflict(task)) continue;
                    if (task.requires_main_thread && !thread_id::is_main_thread()) continue;

                    for (const auto* blacklisted : task.blacklisted) task_data.at(blacklisted).blacklisted_by_count += 1;
                    ecs_access.add_task_access(task);

                    ++tasks_started;
                    return &task;
                }


                return no_tasks_available {};
            });
        }


        VE_CALLED_CONCURRENTLY void complete_task(const scheduler_info<R>& scheduler, const worker_info& worker, const task_info<R>& task) override {
            with_lock(mutex, [&] {
                const auto& data = task_data.at(&task);
                const auto& task = *data.task;

                for (const auto* blacklisted : task.blacklisted) {
                    task_data.at(blacklisted).blacklisted_by_count -= 1;
                }

                for (const auto* dependent : task.dependents) {
                    auto& dependent_data = task_data.at(dependent);

                    dependent_data.unfulfilled_dependencies -= 1;
                    if (dependent_data.unfulfilled_dependencies == 0) available_tasks.emplace(&dependent_data);
                }

                ecs_access.remove_task_access(task);
            });
        }
    private:
        struct task_additional_data {
            const task_info<R>* task;
            time::duration priority;
            std::size_t unfulfilled_dependencies;
            std::size_t blacklisted_by_count;
        };


        /** Comparator for sorting tasks in order of descending priority. */
        struct priority_comparator {
            [[nodiscard]] bool operator()(const task_additional_data* lhs, const task_additional_data* rhs) const {
                return lhs->priority > rhs->priority;
            }
        };


        std::mutex mutex;
        access_tracker ecs_access;
        hash_map<const task_info<R>*, task_additional_data> task_data;
        tree_set<task_additional_data*, priority_comparator> available_tasks;
        std::size_t tasks_started;


        void create_task_data(const task_info<R>& task) const {
            time::duration priority = task.performance;

            for (const auto& dependent : task.dependents) {
                if (!task_data.contains(dependent)) create_task_data(*dependent);
                priority += task_data.at(dependent).priority;
            }

            auto [it, success] = task_data.emplace(
                &task,
                task_additional_data {
                    .task                     = &task,
                    .priority                 = priority,
                    .unfulfilled_dependencies = task.dependencies.size(),
                    .blacklisted_by_count     = 0
                }
            );

            if (it->second.unfulfilled_dependencies == 0) available_tasks.emplace(&(it->second));
        }
    };
}