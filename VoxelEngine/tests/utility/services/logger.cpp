#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/utility/services/logger.hpp>

#include <thread>
#include <atomic>
#include <sstream>
#include <string>
#include <string_view>


constexpr std::size_t test_num_threads  = 128;
constexpr std::size_t test_num_messages = 64;


TEST(logger, multiple_targets) {
    std::stringstream a, b, c;

    ve::logger logger { "logger", ve::logger::DEBUG, a, b, c };
    logger << "This is a message";

    ASSERT_EQ(a.str(), "This is a message");
    ASSERT_EQ(b.str(), "This is a message");
    ASSERT_EQ(c.str(), "This is a message");
}


TEST(logger, asynchronous_access) {
    std::stringstream target;
    ve::logger logger { "logger", ve::logger::DEBUG, target };


    std::vector<std::thread> threads;
    std::atomic_uint64_t ready = 0;

    for (std::size_t i = 0; i < test_num_threads; ++i) {
        threads.emplace_back([&] {
            ++ready;
            while (ready != test_num_threads) std::this_thread::yield();

            for (std::size_t j = 0; j < test_num_messages; ++j) {
                logger.atomically([] (ve::logger& log) {
                    log << "This is " << "a" << " message!" << "\n";
                });
            }
        });
    }


    for (auto& thread : threads) thread.join();

    std::string result_string    = target.str();
    std::string_view view        = result_string;
    std::string expect_substring = "This is a message!\n";

    for (std::size_t i = 0; i < test_num_threads * test_num_messages; ++i) {
        ASSERT_TRUE(view.starts_with(expect_substring));
        view.remove_prefix(expect_substring.length());
    }

    ASSERT_TRUE(view.empty());
}