#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>
#include <VoxelEngine/utility/services/random.hpp>
#include <VoxelEngine/utility/services/logger.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>

#include <boost/icl/interval_map.hpp>
#include <boost/icl/continuous_interval.hpp>
#include <gtest/gtest.h>

#include <functional>
#include <tuple>
#include <type_traits>
#include <thread>
#include <atomic>
#include <chrono>


namespace ve::testing {
    /**
     * Utility for performing fuzz-tests on an object with both randomized data and function calls.
     * The operation fuzzer can be provided with a set of operations that act upon some state, and will then randomly invoke a said amount of these operations.
     * @tparam State The state (at least the object being tested) passed to each operation.
     */
    template <typename State> class operation_fuzzer {
    public:
        operation_fuzzer(void) = default;
        VE_IMMOVABLE(operation_fuzzer);


        ~operation_fuzzer(void) {
            // Note: boost::icl is a C++03 library so move-only types like std::unique_ptr do not work (Container will try to copy them).
            for (const auto& [interval, pointer] : operations) delete pointer;
        }


        /**
         * Add a new operation that will be invoked during fuzz testing. The operation should be invokable as f(State&, const invoke_result_t<Args>&...).
         * @param operation The operation that will be added to the operation_fuzzer.
         * @param weight A weight for how often to invoke the operation compared to other added operations.
         * @param parameters A set of generators for fuzz-values to pass to the operation. Each generator should return a random valid argument each time it is invoked. See @ref fuzz_arguments.
         */
        template <typename... Args> void add_operation(std::function<void(State&, const std::invoke_result_t<Args>&...)> operation, float weight, Args&&... parameters) {
            operations.insert(std::make_pair(
                boost::icl::continuous_interval<float>::right_open(weight_sum, weight_sum + weight),
                // Note: boost::icl is a C++03 library so move-only types like std::unique_ptr do not work (Container will try to copy them).
                (operation_data_base*) new operation_data<Args...> {
                    std::move(operation),
                    std::tuple<Args...> { fwd(parameters)... }
                }
            ));

            weight_sum += weight;
        }


        /**
         * Runs the operation fuzzer, invoking 'operation_count' random operations from the list of previously added operations.
         * @param initial_state The initial value of the state passed to operations.
         * @param operation_count The number of operations to invoke.
         * @param thread_count The number of threads to run operations on.
         */
        void invoke(State initial_state = State { }, std::size_t operation_count = 1e6, std::size_t thread_count = 1) const {
            VE_ASSERT(!operations.empty(), "The operation_fuzzer was invoked without any operations.");
            VE_ASSERT(weight_sum > 0,      "All operations in the operation_fuzzer have a weight of 0.");
            VE_ASSERT(thread_count > 0,    "The operation_fuzzer requires at least one thread to run on.");


            auto& seeder_rng = get_service<random::fast_random_generator>();

            const std::size_t tasks_per_thread = (operation_count / thread_count) + 1;
            std::vector<std::thread> threads;
            std::atomic_uint64_t threads_ready = 0;

            for (std::size_t i = 0; i < thread_count; ++i) {
                threads.emplace_back([&, i, seed = seeder_rng.random_int<u64>()] {
                    // Wait for all threads to be ready.
                    ++threads_ready;
                    while (threads_ready != thread_count) std::this_thread::yield();

                    // Create a thread-local RNG since the global one is not thread-safe.
                    auto local_rng = random::random_number_generator<random::xorshift_generator>();
                    local_rng.reseed(seed);

                    // Keep track of time elapsed.
                    auto start_time = std::chrono::steady_clock::now();

                    // Invoke operations.
                    const std::size_t starting_task = (i * tasks_per_thread);
                    const std::size_t end_task      = std::min((i + 1) * tasks_per_thread, operation_count);

                    for (std::size_t j = starting_task; j < end_task; ++j) {
                        auto it = operations.find(local_rng.random_real(0.0f, weight_sum));
                        std::invoke(*(it->second), initial_state);

                        // Print progress message.
                        if ((j + 1) % (tasks_per_thread / 10) == 0) {
                            const auto [current_num, current_sfx] = ve::make_si_suffixed(j - starting_task + 1);
                            const auto [total_num, total_sfx]     = ve::make_si_suffixed(end_task - starting_task);

                            get_service<engine_logger>().info(
                                "[Fuzzer Thread {}: {}/{}] (Took {:%T})",
                                i,
                                ve::make_si_suffixed_string(j - starting_task + 1, 2),
                                ve::make_si_suffixed_string(end_task - starting_task, 2),
                                std::chrono::round<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
                            ) << std::flush;
                        }
                    }
                });
            }

            for (auto& thread : threads) thread.join();
        }
    private:
        struct operation_data_base;
        boost::icl::interval_map<float, operation_data_base*> operations;
        float weight_sum = 0;


        struct operation_data_base {
            virtual ~operation_data_base(void) = default;
            virtual void operator()(State& state) const = 0;
        };


        template <typename... Args> struct operation_data : operation_data_base {
            std::function<void(State&, const std::invoke_result_t<Args>&...)> operation;
            std::tuple<Args...> arguments;


            operation_data(decltype(operation) op, decltype(arguments) args) :
                operation(std::move(op)),
                arguments(std::move(args))
            {}


            void operator()(State& state) const override {
                std::apply([&] (const auto&... args) {
                    std::invoke(operation, state, std::invoke(args)...);
                }, arguments);
            }
        };
    };


    template <typename State, typename... Args> void fuzz_single_operation(
        std::function<void(State&, const std::invoke_result_t<Args>&...)> operation,
        std::tuple<Args...> parameters,
        State initial_state         = State {},
        std::size_t operation_count = 1e6,
        std::size_t thread_count    = 1
    ) {
        operation_fuzzer<State> fuzzer;

        std::apply([&] (auto&&... args) {
            fuzzer.add_operation(std::move(operation), 1.0f, std::move(args)...);
        }, std::move(parameters));

        fuzzer.invoke(initial_state, operation_count, thread_count);
    }
}