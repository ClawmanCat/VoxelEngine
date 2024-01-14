#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/graph/graph.hpp>
#include <VoxelEngine/ecs/system/scheduling2/task_polymorphic_wrapper.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>

#include <barrier>


namespace ve::ecs::schedule {
    namespace detail {
        template <typename R> struct barrier_task : task_polymorphic_wrapper<R> {
            explicit barrier_task(std::barrier<no_op_t>* barrier) :
                task_polymorphic_wrapper<R>(type_id<barrier_task<R>>()),
                barrier(barrier)
            {}

            virtual void operator()(R*, time::duration, time::tick_timestamp) {
                barrier->arrive_and_wait();
            }

            std::barrier<no_op_t>* barrier;
        };


        template <typename R> task_info<R> make_barrier_task(std::barrier<no_op_t>* barrier) {
            return task_info<R> {
                .task                      = make_unique<barrier_task<R>>(barrier),
                .id                        = max_value<system_id>,
                .requires_main_thread      = false,
                .requires_exclusive_access = false,
                .performance               = time::duration { 1 }
            };
        }
    }


    /**
     * Strategy that prefills a number of buckets equal to the number of workers with roughly equal amounts of work.
     * Barrier tasks are inserted as synchronisation points to prevent tasks from being executed before their dependencies.
     * Optimized for situations where there are many systems with relatively few internal dependencies.
     */
    template <typename R> class prefilled_equal_buckets : public scheduling_strategy<R> {
    public:
        void on_tick_start(const scheduler_info<R>& scheduler) override {
            const std::size_t worker_count = scheduler.scheduler->get_worker_count();

            create_barriers(worker_count);
            create_task_buckets(scheduler, worker_count);
        }


        VE_CALLED_CONCURRENTLY maybe_task<R> try_start_task(const scheduler_info<R>& scheduler, const worker_info& worker) override {
            auto& bucket = buckets[worker.worker_index];

            if (bucket.next_task == bucket.tasks.size()) return no_tasks_remaining {};
            else return bucket.tasks[bucket.next_task++];
        }
    private:
        struct task_data {
            task_data(void) = default;

            explicit task_data(std::size_t workers) :
                barrier(make_unique<std::barrier<no_op_t>>((std::ptrdiff_t) workers)),
                task(detail::make_barrier_task<R>(barrier.get()))
            {}

            unique<std::barrier<no_op_t>> barrier;
            task_info<R> task;
        };


        struct worker_data {
            std::vector<const task_info<R>*> tasks;
            std::size_t next_task = 0;
        };


        std::vector<task_data> barrier_tasks;
        std::vector<worker_data> buckets;


        void create_barriers(std::size_t worker_count) {
            const std::size_t barrier_count = barrier_tasks.size();


            if (barrier_count != worker_count) {
                barrier_tasks.resize(worker_count);

                for (std::size_t i = barrier_count; i < worker_count; ++i) {
                    auto barrier = make_unique<std::barrier<no_op_t>>(worker_count);
                    auto address = barrier.get();

                    barrier_tasks[i] = task_data {
                        .barrier = std::move(barrier),
                        .task    = detail::make_barrier_task<R>(address)
                    };
                }
            }
        }


        void create_task_buckets(const scheduler_info<R>& scheduler, std::size_t worker_count) {

        }
    };
}