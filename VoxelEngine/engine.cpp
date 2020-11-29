#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/engine_event.hpp>
#include <VoxelEngine/threading/threadsafe.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/dependent/plugin_manager.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/io/paths.hpp>
#include <VoxelEngine/utils/io/io.hpp>
#include <VoxelEngine/graphics/window_manager.hpp>
#include <VoxelEngine/graphics/layerstack.hpp>
#include <VoxelEngine/input/input_manager.hpp>

#include <SDL.h>
#include <magic_enum.hpp>

#include <exception>
#include <filesystem>


namespace ve {
    const actor_id engine::engine_actor_id = next_actor_id();
    const version  engine::engine_version  = {
        "PreAlpha",
        VOXELENGINE_VERSION_MAJOR,
        VOXELENGINE_VERSION_MINOR,
        VOXELENGINE_VERSION_PATCH
    };
    
    
    [[noreturn]] void engine::main(u32 argc, char** argv) noexcept {
        engine::args = std::vector<std::string> { argv, argv + argc };
        
        
        try {
            while (true) {
                static engine::state prev_state = engine::engine_state;
                if (engine::engine_state != prev_state) {
                    dispatcher.dispatch_event(engine_state_change_event {
                        prev_state,
                        engine::engine_state
                    });
                    
                    prev_state = engine::engine_state;
                }
                
                switch (engine::engine_state) {
                    case state::UNINITIALIZED: {
                        engine::on_init();
                        break;
                    }
                    case state::RUNNING: {
                        engine::on_loop();
                        break;
                    }
                    case state::EXITING: {
                        engine::on_exit();
                        break;
                    }
                    default:
                        VE_LOG_FATAL(
                            "The engine was in an invalid state and will be terminated: "s +
                            std::string(magic_enum::enum_name(engine::engine_state))
                        );
                        
                        throw std::runtime_error("Invalid engine state");
                }
            }
        } catch (const std::exception& e) {
            VE_LOG_FATAL("Uncaught exception: "s + e.what());
        } catch (...) {
            VE_LOG_FATAL("Uncaught exception. No further information is available.");
        }
    
        dispatcher.dispatch_event(engine_uncaught_error_event { });
        engine::exit(-1, true);
        engine::on_exit(); // Unreachable, but suppresses warning.
    }
    
    
    void engine::exit(i32 exit_code, bool immediate) {
        ve_threadsafe_function;
        
        VE_LOG_INFO((immediate ? "Immediate"s : "Delayed"s) + " exit requested with code " + std::to_string(exit_code) + ".");
        engine::engine_state = state::EXITING;
        engine::exit_code    = exit_code;
        
        if (immediate) dispatcher.dispatch_event(engine_immediate_exit_event { });
        else dispatcher.dispatch_event(engine_delayed_exit_event { });
        
        if (immediate) engine::on_exit();
    }
    
    
    void engine::on_init(void) {
        game_callbacks::on_actor_id_provided(next_actor_id());
        game_callbacks::on_game_pre_init();
        dispatcher.dispatch_event(engine_pre_init_event { });
        
        engine::engine_state = state::INITIALIZING;
        VE_LOG_INFO("Beginning initialization phase...");
        
        // Create required directories.
        io::create_required_paths();
        
        // Initialize libraries.
        SDL_SetMainReady();
        SDL_Init(SDL_INIT_EVERYTHING);
        
        // Create window.
        // TODO: Replace this with event.
        window_manager::instance().create(game_callbacks::get_game_info()->name.c_str());
        
        // Load plugins.
        for (const auto& elem : fs::directory_iterator(io::paths::PATH_PLUGINS)) {
            if (elem.is_regular_file() && elem.path().extension() == library_extension) {
                plugin_manager::instance().load_plugin(elem.path(), false);
            }
        }
        
        engine::engine_state = state::RUNNING;
        VE_LOG_INFO("Finished initialization phase...");
        
        game_callbacks::on_game_post_init();
        dispatcher.dispatch_event(engine_post_init_event { });
    }
    
    
    void engine::on_loop(void) {
        game_callbacks::on_game_pre_loop();
        dispatcher.dispatch_event(engine_pre_loop_event { });
        
        input_manager::instance().update(engine::tick_count);
        
        window_manager::instance().on_frame_start();
        layerstack::instance().draw();
        window_manager::instance().on_frame_end();
        
        game_callbacks::on_game_post_loop();
        dispatcher.dispatch_event(engine_post_loop_event { });
        
        ++engine::tick_count;
    }
    
    
    [[noreturn]] void engine::on_exit(void) {
        try {
            game_callbacks::on_game_pre_exit();
            dispatcher.dispatch_event(engine_pre_exit_event { });
    
            VE_LOG_INFO("Beginning exit phase...");
    
    
            // Unload plugins.
            plugin_manager::instance().unload_all_plugins(false);
            VE_ASSERT(plugin_manager::instance().get_loaded_plugins().size() == 0);
    
    
            engine::engine_state = state::EXITED;
            VE_LOG_INFO("Finished exit phase...");
    
            game_callbacks::on_game_post_exit();
            dispatcher.dispatch_event(engine_post_exit_event { });
        } catch (...) {
            VE_LOG_ERROR("An error occurred during engine termination. The engine may not have exited correctly.");
        }
        
        std::exit(engine::exit_code.value_or(-1));
    }
}