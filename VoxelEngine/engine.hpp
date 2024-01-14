#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vector>
#include <string>


namespace ve {
    /** Class to manage the global state of the engine. */
    class engine {
    public:
        enum engine_state { UNINITIALIZED, INITIALIZING, RUNNING, STOP_REQUESTED, UNINITIALIZED_STOP_REQUESTED, STOPPING, STOPPED };


        VE_SERVICE(engine);


        /**
         * Starts the game engine. This function returns if engine::stop is called.
         * @param args Command line arguments.
         */
        void start(std::vector<std::string> args);


        /**
         * Causes the current invocation of engine::start to return after finishing the current tick.
         * After returning, the engine is in an uninitialized state, essentially as it was before calling engine::start.
         */
        void stop(void);


        /**
         * Stops the engine and exits the program with the given exit code, either immediately or after the current tick.
         * Note that this will not cause engine::start to return. If this is the desired behaviour, use engine::stop instead.
         * @param exit_code Exit code to exit the program with. Non-zero exit codes should indicate some error occurred.
         * @param immediate If true, the program will exit before returning from this function. If false, it will exit after finishing the current tick.
         */
        void exit(i32 exit_code = EXIT_SUCCESS, bool immediate = false);


        VE_GET_CREFS(arguments);
        VE_GET_VALS(state, exit_code);
    private:
        engine_state state = UNINITIALIZED;
        std::vector<std::string> arguments = { };
        std::optional<i32> exit_code = std::nullopt;

        void engine_init(void);
        void engine_loop(void);
        void engine_exit(void);

        void set_state(engine_state state);
    };
}