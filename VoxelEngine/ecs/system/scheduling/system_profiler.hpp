#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>
#include <VoxelEngine/ecs/system/system.hpp>

#include <range/v3/algorithm.hpp>
#include <range/v3/numeric.hpp>

#include <chrono>


namespace ve::ecs::schedule {
    /**
     * Wrapper around an ECS system to track its performance over time, used to manage its scheduling priority.
     * @tparam System The type of the system to track.
     */
    template <ecs_system System> class system_profiler {
    public:
        explicit system_profiler(System system, std::size_t num_samples = 16) : system(std::move(system)) {
            set_num_samples(num_samples);
        }


        /**
         * Invokes the system and returns its average runtime over the duration of the performance window.
         * @param args Arguments to invoke the system with.
         * @return The average performance of the system over the last 'num_samples' invocations.
         */
        template <typename... Args> time::duration operator()(Args&&... args) {
            auto start = time::clock::now();
            std::invoke(system, fwd(args)...);
            auto end = time::clock::now();

            update_performance_data(end - start);
            return average;
        }


        /**
         * Sets the number of performance samples that are averaged over to measure the system performance.
         * @param count The number of performance samples to store.
         */
        void set_num_samples(std::size_t count) {
            VE_DEBUG_ASSERT(count > 0, "Number of performance samples should be greater than 0.");

            samples.resize(count);

            was_invoked = false;
            update_performance_data(average);
        }


        VE_GET_MREFS(system);
        VE_GET_VALS(average);
    private:
        System system;

        std::vector<time::duration> samples;
        time::duration average { 1 };

        bool was_invoked = false;
        std::size_t sample_index = 0;


        void update_performance_data(time::duration dt) {
            if (was_invoked) [[likely]] {
                if (sample_index >= samples.size()) sample_index = 0;

                samples[sample_index++] = dt;
                average = ranges::accumulate(samples, time::duration { 0 }) / samples.size();
            } else {
                ranges::fill(samples, dt);
                average = dt;
            }

            was_invoked = true;
        }
    };
}