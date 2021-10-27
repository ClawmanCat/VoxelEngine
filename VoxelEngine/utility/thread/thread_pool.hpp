#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <boost/asio.hpp>

#include <thread>
#include <mutex>
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
            return boost::asio::post(pool, fwd(task), boost::asio::use_future);
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
            return boost::asio::post(main_thread_ctx, fwd(task), boost::asio::use_future);
        }
    
        template <typename Task>
        void invoke_on_main_and_await(Task&& task) {
            VE_ASSERT(std::this_thread::get_id() != main_thread_id, "Cannot await main thread on main thread!");
            invoke_on_main_with_future(fwd(task)).wait();
        }
    private:
        const static inline auto main_thread_id = std::this_thread::get_id();
        
        boost::asio::thread_pool pool;
        boost::asio::io_context main_thread_ctx;
        
        
        explicit thread_pool(std::size_t num_workers = std::max(16u, std::thread::hardware_concurrency()))
            : pool(num_workers) {}
        
        
        friend class engine;
        void execute_main_thread_tasks(void) {
            main_thread_ctx.run();
        }
    };
}