#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>

#include <boost/asio.hpp>
#include <VoxelEngine/core/windows_header_cleanup.hpp>

#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <queue>


namespace ve {
    class thread_pool {
    public:
        static thread_pool& instance(void);
        
            
        ~thread_pool(void) {
            // Finish any remaining tasks.
            pool.join();
            main_thread_ctx.run();
        }
        
        
        template <typename Task>
        void invoke_on_thread(Task&& task) {
            boost::asio::post(pool, fwd(task));
        }
    
        template <typename Task>
        auto invoke_on_thread_with_future(Task&& task) {
            using task_t = std::packaged_task<typename meta::function_traits<Task>::signature>;
            return boost::asio::post(pool, task_t { fwd(task) });
        }
    
        template <typename Task>
        void invoke_on_thread_and_await(Task&& task) {
            invoke_on_thread_with_future(fwd(task)).wait();
        }
        
    
        template <typename Task>
        void invoke_on_main(Task&& task) {
            boost::asio::post(main_thread_ctx, fwd(task));
        }
        
        template <typename Task>
        auto invoke_on_main_with_future(Task&& task) {
            using task_t = std::packaged_task<typename meta::function_traits<Task>::signature>;
            return boost::asio::post(main_thread_ctx, task_t { fwd(task) });
        }
    
        template <typename Task>
        void invoke_on_main_and_await(Task&& task) {
            VE_ASSERT(!is_main_thread(), "Cannot await main thread on main thread!");
            invoke_on_main_with_future(fwd(task)).wait();
        }

        // Similar to above, but if we're already on the main thread, the task is executed immediately.
        template <typename Task>
        void invoke_on_main_or_run(Task&& task) {
            if (is_main_thread()) {
                std::invoke(task);
            } else {
                invoke_on_main_with_future(fwd(task)).wait();
            }
        }

        bool is_main_thread(void) const {
            return std::this_thread::get_id() == main_thread_id;
        }
    private:
        const static inline auto main_thread_id = std::this_thread::get_id();
        
        boost::asio::thread_pool pool;
        boost::asio::io_context main_thread_ctx;
        
        
        explicit thread_pool(std::size_t num_workers = std::max(32u, std::thread::hardware_concurrency()))
            : pool(num_workers) {}
        
        
        friend class engine;
        void execute_main_thread_tasks(void) {
            main_thread_ctx.run();
        }
    };
}