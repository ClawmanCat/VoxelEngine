#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>

#include <range/v3/numeric.hpp>
#include <boost/circular_buffer.hpp>


namespace ve::ecs::schedule {
    /** Generic wrapper around a task to profile its average performance. */
    template <typename Task> class task_profiling_wrapper {
    public:
        explicit task_profiling_wrapper(Task task, std::size_t performance_samples = 16) :
            task(std::move(task)),
            average_performance(time::duration { 1 })
        { set_sample_count(performance_samples); }


        template <typename... Args> void operator()(Args&&... args) {
            auto start = time::clock::now();
            std::invoke(task, fwd(args)...);
            auto stop  = time::clock::now();

            performance_samples.push_back(stop - start);
            average_performance = ranges::accumulate(performance_samples, time::duration { 0 }) / performance_samples.size();
        }


        void set_sample_count(std::size_t count) {
            performance_samples.resize(count, average_performance);
        }

        [[nodiscard]] std::size_t get_sample_count(void) const {
            return performance_samples.max_size();
        }


        VE_GET_MREFS(task);
        VE_GET_VALS(average_performance);
    private:
        Task task;
        boost::circular_buffer<time::duration> performance_samples;
        time::duration average_performance;
    };
}