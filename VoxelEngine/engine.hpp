#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/version.hpp>
#include <VoxelEngine/event/event_dispatcher.hpp>
#include <VoxelEngine/engine_state.hpp>

#include <vector>
#include <string>
#include <string_view>


namespace ve {
    class engine {
    public:
        [[noreturn]] static void main(i32 argc, char** argv);
        static void request_exit(i32 exit_code = 0, bool immediate = false);
    
        VE_GET_STATIC_CREF(engine_version)
        VE_GET_STATIC_CREF(args)
        VE_GET_STATIC_MREF(dispatcher)
        VE_GET_STATIC_VAL(state)
        VE_GET_STATIC_VAL(tick_count)
        
        VE_GET_SET_STATIC_VAL(client_target_dt)
        VE_GET_SET_STATIC_VAL(server_target_dt)
    private:
        static inline version engine_version = { "PreAlpha", 0, 0, 1 };
        static inline std::vector<std::string> args = {};
        
        static inline noncancellable_event_dispatcher dispatcher;
        static inline engine_state state = engine_state::UNINITIALIZED;
        static inline optional<i32> exit_code = nullopt;
        
        static inline u64 tick_count = 0;
        static inline microseconds client_target_dt = (1s / 60);
        static inline microseconds server_target_dt = (1s / 60);
        
        
        static void init(void);
        static void tick(void);
        [[noreturn]] static void exit(void);
        
        static void set_state(engine_state state);
    };
}