#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/event/immediate_prioritized_event_dispatcher.hpp>

#include <vector>
#include <string>
#include <optional>
#include <cstdlib>


namespace ve {
    class engine {
    public:
        using dispatcher_t = events::immediate_prioritized_event_dispatcher<>;
        
        static const actor_id engine_actor_id;
        static const version  engine_version;
        
        
        enum class state {
            UNINITIALIZED, INITIALIZING, RUNNING, EXITING, EXITED
        };
        
        
        // Run the engine with the given command line parameters.
        [[noreturn]] static void main(u32 argc, char** argv) noexcept;
        
        // Stops the engine.
        // If immediate is set to true, the engine will exit immediately, and this function does not return.
        // If immediate is set to false, the engine will exit after the current loop finishes.
        static void exit(i32 exit_code = 0, bool immediate = false);
        
        
        [[nodiscard]] static const std::vector<std::string>& get_args(void) noexcept {
            return engine::args;
        }
        
        [[nodiscard]] static engine::state get_state(void) noexcept {
            return engine::engine_state;
        }
        
        [[nodiscard]] static u64 get_tick_count(void) noexcept {
            return engine::tick_count;
        }
        
        [[nodiscard]] static dispatcher_t& get_dispatcher(void) noexcept {
            return dispatcher;
        }
    private:
        static inline std::vector<std::string> args = {};
        static inline state engine_state            = state::UNINITIALIZED;
        static inline std::optional<i32> exit_code  = std::nullopt;
        static inline u64 tick_count                = 0;
        static inline dispatcher_t dispatcher       = {};
        
        
        static void on_init(void);
        static void on_loop(void);
        
        [[noreturn]] static void on_exit(void);
    };
}