#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/utility/container/visitor.hpp>
#include <VoxelEngine/utility/threading/threading_utils.hpp>
#include <VoxelEngine/utility/threading/mutexed.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy_selector.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/scheduler_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/worker_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>

#include <condition_variable>
#include <mutex>


namespace ve::ecs::schedule {
    /**
     * The system scheduler executes tasks given to it according to some provided strategy.
     * The strategy used by the scheduler can be changed dynamically by the provided strategy_selector.
     */
    template <typename Registry> class system_scheduler {
    public:
        constexpr static inline std::size_t main_thread_worker_index = 0;


        /**
         * Constructs a new system scheduler.
         * @param strategy_selector Object used to control the task execution strategy.
         * @param initial_strategy Initial task execution strategy. This parameter may be null, in which case the strategy is retrieved from the strategy selector.
         * @param num_workers Number of worker threads used by the scheduler. Defaults to the number of threads on the current system.
         */
        explicit system_scheduler(
            unique<scheduling_strategy_selector<Registry>> strategy_selector,
            unique<scheduling_strategy<Registry>> initial_strategy = nullptr,
            std::size_t num_workers = std::thread::hardware_concurrency()
        ) :
            info { .scheduler = this },
            strategy(std::move(initial_strategy)),
            strategy_selector(std::move(strategy_selector))
        {
            VE_ASSERT(strategy_selector, "Strategy selector may not be null.");

            if (!strategy) {
                update_strategy(strategy_change_reason::SCHEDULER_CONSTRUCTED);
                VE_ASSERT(strategy, "Cannot create system_scheduler without initial scheduling strategy.");
            }


            workers.push_back({ .thread = std::nullopt, .worker_index = main_thread_worker_index });

            for (std::size_t i = 0; i < num_workers; ++i) {
                workers.push_back({
                    .thread       = std::thread { &system_scheduler::worker_main, this, i + 1 },
                    .worker_index = i + 1
                });
            }
        }


        ~system_scheduler(void) {
            with_lock(signalling_data.mutex, [&] { signalling_data.exit_workers = true; });
            signalling_data.signal_wake_workers.notify_all();

            for (auto& worker : workers) {
                if (worker.thread) worker.thread->join();
            }
        }


        // Worker threads receive pointer to the parent scheduler so don't allow moving it.
        VE_IMMOVABLE(system_scheduler);


        /**
         * Invokes the scheduler, executing all tasks using the current task execution strategy.
         * @param registry The registry this scheduler is part of.
         * @param dt Amount of time simulated this tick.
         * @param now Timestamp of the current tick.
         */
        void operator()(Registry* registry, time::duration dt, time::tick_timestamp now) {
            tick_data = tick_data_t {
                .registry = registry,
                .dt       = dt,
                .now      = now
            };

            signalling_data.busy_workers = workers.size() - 1;


            strategy->on_tick_start(info);

            // Wake workers.
            with_lock(signalling_data.mutex, [&] { signalling_data.scheduler_invoked = true; });
            signalling_data.signal_wake_workers.notify_all();

            // Do work and wait for workers to finish.
            while (do_work(main_thread_worker_index));
            while (signalling_data.busy_workers != 0) std::this_thread::yield();

            // Collect new performance metrics.
            for (auto& [id, task] : info.tasks) task.performance = task.task->get_performance();

            strategy->on_tick_completed(info);
            update_strategy(strategy_change_reason::SCHEDULER_TICKED);
        }


        /**
         * Assigns an unique ID to the given system and adds it to the scheduler.
         * @param registry The registry this scheduler is part of.
         * @param system The system to add to the scheduler.
         * @return The ID of the system.
         */
        template <ecs_system System> [[nodiscard]] system_id add_system(Registry* registry, System system) {
            system_id id = next_id++;
            info.add_task(registry, task_info<Registry>::from_traits(std::move(system), id));

            update_strategy(strategy_change_reason::SYSTEM_ADDED, info.tasks.at(id));
            return id;
        }


        /**
         * Removes an existing system from the scheduler.
         * @param registry The registry this scheduler is part of.
         * @param system The ID of the system to remove from the scheduler.
         * @return True if the system was removed or false if no such system existed.
         */
        bool remove_system(Registry* registry, system_id system) {
            if (auto it = info.tasks.find(system); it != info.tasks.end()) {
                update_strategy(strategy_change_reason::SYSTEM_REMOVED, std::addressof(it.second));
                info.remove_task(registry, it->second);

                return true;
            }

            return false;
        }


        /** Returns true if a system with the given ID exists, or false otherwise. */
        [[nodiscard]] bool has_system(system_id system) const {
            return info.tasks.contains(system);
        }


        /** Returns true if a system with the given ID and type exists, or false otherwise. */
        template <ecs_system System> [[nodiscard]] bool has_system(system_id system) const {
            if (auto it = info.tasks.find(system); it != info.tasks.end()) {
                if (const auto* task = it->second.task->template get_task<System>(); task) return true;
            }

            return false;
        }


        /**
         * Returns a pointer to the system with the given ID if it exists and has the provided type, or null otherwise.
         * @tparam System The type of the system.
         * @param system The ID of the system.
         * @return A pointer to the system if it exists and has the given type, or null otherwise.
         */
        template <ecs_system System> [[nodiscard]] System* get_system(system_id system) {
            if (auto it = info.tasks.find(system); it != info.tasks.end()) {
                if (auto* task = it->second.task->template get_task<System>(); task) return task;
            }

            return nullptr;
        }


        /** @copydoc get_system */
        template <ecs_system System> [[nodiscard]] const System* get_system(system_id system) const {
            if (auto it = info.tasks.find(system); it != info.tasks.end()) {
                if (const auto* task = it->second.task->template get_task<System>(); task) return task;
            }

            return nullptr;
        }


        /**
         * Removes and returns the system with the given ID and type from the scheduler.
         * @tparam System The type of the system.
         * @param registry The registry this scheduler is part of.
         * @param system The ID of the system.
         * @return The system as an std::optional, if a system with the given type and ID was present, or std::nullopt otherwise.
         * @post The system is not present in the scheduler after calling this method.
         */
        template <ecs_system System> [[nodiscard]] std::optional<System> take_system(Registry* registry, system_id system) {
            if (auto it = info.tasks.find(system); it != info.tasks.end()) {
                if (auto* task = it->second.task->template get_task<System>(); task) {
                    update_strategy(strategy_change_reason::SYSTEM_REMOVED, std::addressof(it.second));
                    task->on_removed(registry);

                    std::optional<System> result = std::move(*task);
                    info.tasks.erase(it);
                    return result;
                }
            }

            return std::nullopt;
        }


        [[nodiscard]] std::size_t get_worker_count(void) const { return workers.size(); }
    private:
        /** Unprotected object for internal use only. */
        scheduler_info<Registry> info;

        std::vector<worker_info> workers;

        unique<scheduling_strategy<Registry>> strategy;
        unique<scheduling_strategy_selector<Registry>> strategy_selector;

        system_id next_id = 0;


        struct signalling_data_t {
            std::condition_variable signal_wake_workers;
            std::atomic_uint32_t busy_workers;
            std::mutex mutex;
            bool scheduler_invoked = false, exit_workers = false;
        } signalling_data;


        struct tick_data_t {
            Registry* registry;
            time::duration dt;
            time::tick_timestamp now;
        } tick_data;


        /** Main function for worker threads. Function returns after setting exit_workers and signalling signal_wake_workers. */
        void worker_main(std::size_t worker_index) {
            while (true) {
                signalling_data.signal_wake_workers.wait([&] {
                    return signalling_data.scheduler_invoked || signalling_data.exit_workers;
                });

                if (with_lock(signalling_data.mutex, [&] { signalling_data.exit_workers; })) return;

                while (do_work(worker_index));
                --signalling_data.busy_workers;
            }
        }


        /**
         * Executes a task if one is available.
         * Returns true if there are no more tasks that can be done this tick by this thread, or false otherwise.
         */
        bool do_work(std::size_t worker_index) {
            auto task = strategy->try_start_task(info, workers[worker_index]);
            bool done = false;


            std::visit(make_visitor(
                [&] (no_tasks_available) { std::this_thread::yield(); },
                [&] (no_tasks_remaining) { done = true; },
                [&] (task_info<Registry>* task) {
                    std::invoke(*task, tick_data.registry, tick_data.dt, tick_data.now);
                    strategy->complete_task(info, workers[worker_index], *task);
                }
            ), task);


            return done;
        }


        /** Checks if there is a new strategy, and switches to it if so. */
        void update_strategy(strategy_change_reason reason, const task_info<Registry>* task = nullptr) {
            auto new_strategy = strategy_selector->update_strategy(info, reason, task);

            if (new_strategy) {
                strategy = std::move(new_strategy);
            }
        }
    };
}