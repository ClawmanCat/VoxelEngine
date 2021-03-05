#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/engine_event.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/thread/make_nonconcurrent.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/input/input_manager.hpp>

#include <VoxelEngine/graphics/graphics.hpp>
#include graphics_include(window/window_registry.hpp)

#include <magic_enum.hpp>


namespace ve {
    void engine::set_state(engine_state state) {
        engine_state prev_state = std::exchange(engine::state, state);
        
        dispatcher.dispatch_event(engine_state_change {
            .old_state = prev_state,
            .new_state = engine::state
        });
    }
    
    
    void engine::main(i32 argc, char** argv) {
        engine::args = std::vector<std::string> { argv, argv + argc };
        
        while (true) {
            VE_ASSERT(
                one_of(engine::state, engine_state::UNINITIALIZED, engine_state::RUNNING, engine_state::EXITING),
                "Invalid engine state: "s + magic_enum::enum_name(engine::state)
            );
            
            switch (engine::state) {
                case engine_state::UNINITIALIZED:
                    engine::init();
                    break;
                case engine_state::RUNNING:
                    engine::tick();
                    break;
                case engine_state::EXITING:
                    engine::exit();
                    break;
                default: VE_UNREACHABLE;
            }
        }
    }
    
    
    void engine::request_exit(i32 exit_code, bool immediate) {
        ve_make_nonconcurrent;
        
        engine::exit_code = exit_code;
        set_state(engine_state::EXITING);
        
        if (immediate) engine::exit();
    }
    
    
    void engine::init(void) {
        game_callbacks::on_game_pre_init();
        set_state(engine_state::INITIALIZING);
        
        // Make sure all required directories exist.
        io::create_required_paths();
        
        set_state(engine_state::RUNNING);
        game_callbacks::on_game_post_init();
    }
    
    
    void engine::tick(void) {
        game_callbacks::on_game_pre_loop();
        dispatcher.dispatch_event(engine_tick_begin { .tick = engine::tick_count });
        
        input_manager::instance().update(engine::tick_count);
        graphics::window_registry::instance().draw_all();
        
        dispatcher.dispatch_event(engine_tick_end { .tick = engine::tick_count });
        ++engine::tick_count;
        game_callbacks::on_game_post_loop();
    }
    
    
    void engine::exit(void) {
        game_callbacks::on_game_pre_exit();
        dispatcher.dispatch_event(engine_exit_begin { });
        
        // Exit code here.
        
        set_state(engine_state::EXITED);
        game_callbacks::on_game_post_exit();
        
        std::exit(engine::exit_code.value_or(-1));
    }
}