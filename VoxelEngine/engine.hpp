#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/arg_parser.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/dependent/dependent_info.hpp>


namespace ve {
    class engine {
    public:
        const static inline game_info info {
            .display_name = "VoxelEngine",
            .description  = { "Game engine for voxel-based games." },
            .authors      = { "ClawmanCat" },
            .version      = { VOXELENGINE_VERSION_MAJOR, VOXELENGINE_VERSION_MINOR, VOXELENGINE_VERSION_PATCH }
        };


        using dispatcher_t = simple_event_dispatcher<false, u16, true>;


        engine(void) = delete;
        
        enum class state { UNINITIALIZED, INITIALIZING, RUNNING, EXITING, EXITED };
        
        [[noreturn]] static void main(i32 argc, char** argv);
        static void exit(i32 code = 0, bool immediate = false);

        
        VE_GET_STATIC_CREF(arguments);
        VE_GET_STATIC_VAL(engine_state);
        VE_GET_SET_STATIC_VAL(tick_count);
        VE_GET_STATIC_MREF(event_dispatcher);
    private:
        static inline arg_parser arguments = arg_parser { };
        static inline state engine_state   = state::UNINITIALIZED;
        static inline u64 tick_count       = 0;
        static inline i32 exit_code        = -1;

        static inline dispatcher_t event_dispatcher = {};
        
        static void init(void);
        static void loop(void);
        [[noreturn]] static void immediate_exit(void);
    };
}