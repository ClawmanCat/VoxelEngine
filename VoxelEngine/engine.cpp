#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/engine_events.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/thread/thread_pool.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/dependent/plugin_registry.hpp>
#include <VoxelEngine/dependent/game_callbacks.hpp>
#include <VoxelEngine/graphics/presentation/window_registry.hpp>
#include <VoxelEngine/input/input_manager.hpp>

#include <SDL.h>


// In debug mode we often want the exception to not be intercepted, so we can see the point where it was thrown.
#ifndef VE_DEBUG
    #define VE_LOG_UNCAUGHT_ERRORS
#endif


namespace ve {
    [[noreturn]] void engine::main(i32 argc, char** argv) {
        engine::arguments.feed((std::size_t) argc, (const char**) argv);


        #ifdef VE_LOG_UNCAUGHT_ERRORS
        try {
        #endif
            while (true) {
                switch (engine::engine_state) {
                    case engine::state::UNINITIALIZED:
                        engine::init();
                        break;
                    case engine::state::RUNNING:
                        engine::loop();
                        break;
                    case engine::state::EXITING:
                        engine::immediate_exit();
                        break;
                    default:
                        throw std::runtime_error("Illegal engine state.");
                }
            }
        #ifdef VE_LOG_UNCAUGHT_ERRORS
        } catch (const std::exception& e) {
            VE_ASSERT(false, "Unhandled exception:", e.what());
        } catch (...) {
            VE_ASSERT(false, "Unhandled exception: no further information.");
        }
        #endif
        
        engine::immediate_exit();
    }
    
    
    void engine::exit(i32 code, bool immediate) {
        engine::event_dispatcher.dispatch_event(engine_exit_requested_event { code });

        engine::exit_code    = code;
        engine::engine_state = engine::state::EXITING;
        
        if (immediate) engine::immediate_exit();
    }
    
    
    void engine::init(void) {
        game_callbacks::pre_init();
        engine::event_dispatcher.dispatch_event(engine_pre_init_event { });

        engine::engine_state = engine::state::INITIALIZING;


        for (const auto& path : io::paths::get_registered_paths()) {
            fs::create_directories(path);
        }


        SDL_SetMainReady();
        SDL_Init(SDL_INIT_EVERYTHING);

        input_manager::instance().add_handler(
            [](const exit_requested_event&) { engine::exit(); },
            priority::LOWEST // Give the game the opportunity to handle this event first.
        );

        
        plugin_registry::instance().scan_folder(io::paths::PATH_PLUGINS);
        plugin_registry::instance().try_load_all_plugins(false);


        engine::engine_state = engine::state::RUNNING;

        engine::event_dispatcher.dispatch_event(engine_post_init_event { });
        game_callbacks::post_init();
    }
    
    
    void engine::loop(void) {
        static nanoseconds last_dt = 1s / 60; // Provide some reasonable fake value for the first tick.
        auto tick_begin = steady_clock::now();

        game_callbacks::pre_loop();
        engine::event_dispatcher.dispatch_event(engine_pre_loop_event { engine::tick_count });

        gfx::window_registry::instance().begin_frame();

        input_manager::instance().update(engine::tick_count);
        thread_pool::instance().execute_main_thread_tasks();
        instance_registry::instance().update_all(last_dt);

        gfx::window_registry::instance().end_frame();

        engine::event_dispatcher.dispatch_event(engine_post_loop_event { engine::tick_count });
        game_callbacks::post_loop();

        ++engine::tick_count;
        last_dt = time_since(tick_begin);
    }
    
    
    [[noreturn]] void engine::immediate_exit(void) {
        game_callbacks::pre_exit();
        engine::event_dispatcher.dispatch_event(engine_pre_exit_event { engine::exit_code });


        // Plugins failing to unload should not jeopardize normal cleanup;
        // attempt to unload all plugins first, then error after cleanup if any remain loaded.
        plugin_registry::instance().try_unload_all_plugins(false);

        VE_ASSERT(
            plugin_registry::instance().get_loaded_plugins().empty(),
            "Some plugins failed to unload while terminating the engine.\n"
            "Exit completed successfully otherwise."
        );


        SDL_Quit();


        engine::engine_state = engine::state::EXITED;

        engine::event_dispatcher.dispatch_event(engine_post_exit_event { engine::exit_code });
        game_callbacks::post_exit();
        
        std::exit(engine::exit_code);
    }
}