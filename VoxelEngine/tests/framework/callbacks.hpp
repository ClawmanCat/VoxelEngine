#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/tests/framework/framework_core.hpp>
#include <VoxelEngine/tests/framework/framework_utils.hpp>


// Call the engine entry point.
int main(int argc, char** argv) {
    ve::engine::main(argc, argv);
}


// Run all tests for a specific stage of the game.
#define VE_IMPL_RUN_STAGE_TESTS(stage)                                  \
bool success = ve::tests::test_registry::instance()                     \
    .run_tests(ve::tests::test_registry::stage);                        \
                                                                        \
if (!success) {                                                         \
    VE_LOG_ERROR("One or more tests failed for stage " #stage ".");     \
}


// Callbacks for the engine to run the tests. This acts as a mock game for the tests to run within.
namespace ve::game_callbacks {
    inline const game_info* get_game_info(void) {
        const static game_info info {
            .name         = "VoxelEngine Tests",
            .description  = { "Automatically generated info struct for VE tests." },
            .authors      = { },
            .game_version = { "Test", 0, 0, 0 }
        };
        
        return &info;
    }
    
    
    inline void on_game_pre_init(void) {
        VE_IMPL_RUN_STAGE_TESTS(PRE_INIT);
    }
    
    inline void on_game_post_init(void) {
        VE_IMPL_RUN_STAGE_TESTS(POST_INIT);
    }
    
    inline void on_game_pre_loop (u64 tick, microseconds dt) {
        VE_IMPL_RUN_STAGE_TESTS(PRE_LOOP);
        if (tick + 1 == tests::test_settings::num_ticks) ve::engine::request_exit(0);
    }
    
    inline void on_game_post_loop(u64 tick, microseconds dt) {
        VE_IMPL_RUN_STAGE_TESTS(POST_LOOP);
    }
    
    inline void on_game_pre_exit(void) {
        VE_IMPL_RUN_STAGE_TESTS(PRE_EXIT);
    }
    
    inline void on_game_post_exit(void) {
        VE_IMPL_RUN_STAGE_TESTS(POST_EXIT);
        
        // Next call would be std::exit anyway, just call it manually first so we can set the return code,
        // in case any of the tests after PRE_LOOP failed.
        tests::test_registry::instance().print_stats();
        std::exit(tests::test_registry::instance().has_errors() ? -1 : 0);
    }
}