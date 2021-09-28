#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/dependent_info.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/engine.hpp>


struct test_result {
    std::optional<std::string> error;
    std::string test_name;

    bool operator==(const test_result& o) const { return error == o.error; }
    bool operator!=(const test_result& o) const { return error != o.error; }
};

#define VE_TEST_SUCCESS test_result { std::nullopt, __FILE__ }
#define VE_TEST_FAIL(...) test_result { ve::cat(__VA_ARGS__), __FILE__ }


extern test_result test_main(void);


namespace ve::game_callbacks {
    void pre_init(void)  { }
    void post_init(void) { }
    void post_loop(void) { }
    void pre_exit(void)  { }
    void post_exit(void) { }
    
    
    void pre_loop(void) {
        auto result = test_main();
        
        if (!result.error) VE_LOG_INFO(cat("Test ", result.test_name, " completed successfully"));
        else VE_LOG_ERROR(cat("Test ", result.test_name, " failed: ", *result.error));
        
        engine::exit(result.error ? EXIT_FAILURE : EXIT_SUCCESS);
    }
    
    
    const game_info* get_info(void) {
        const static game_info info {
            .display_name = "VE Test",
            .description  = { },
            .authors      = { },
            .version      = { 0, 0, 0 }
        };
        
        return &info;
    }
}


// Tests consist of a single translation unit, so this is safe to define here.
int main(int argc, char** argv) {
    ve::engine::main(argc, argv);
}