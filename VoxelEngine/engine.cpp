#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/dependent/game_callbacks.hpp>
#include <VoxelEngine/utility/container/container_utils.hpp>
#include <VoxelEngine/utility/services/logger.hpp>


namespace ve {
    void engine::start(std::vector<std::string> args) {
        arguments = std::move(args);


        while (true) {
            switch (state) {
                case engine::UNINITIALIZED:
                    engine_init();
                    break;
                case engine::RUNNING:
                    engine_loop();
                    break;
                case engine::STOP_REQUESTED:
                    [[fallthrough]];
                case engine::UNINITIALIZED_STOP_REQUESTED:
                    engine_exit();
                    break;
                case engine::STOPPED:
                    set_state(engine::UNINITIALIZED);
                    return;
                default:
                    // TODO: Assert here!
                    VE_UNREACHABLE;
            }
        }
    }


    void engine::stop(void) {
        if (state == engine::STOPPING) {
            get_service<engine_logger>().warning("Engine stop requested while the engine was already stopping. Additional request will be ignored.");
            return;
        }

        set_state(engine::STOP_REQUESTED);
        get_service<engine_logger>().info("Engine stop requested.");
    }


    void engine::exit(i32 exit_code, bool immediate) {
        this->exit_code = exit_code;


        get_service<engine_logger>().info(
            "Program termination requested (With exit code: {}, immediate mode: {}).",
            exit_code,
            immediate
        );


        switch (state) {
            case UNINITIALIZED:
                [[fallthrough]];
            case INITIALIZING:
                // Exit while engine was not initialized: stop but skip exit code.
                state = engine::UNINITIALIZED_STOP_REQUESTED;
                if (immediate) engine_exit();
            case STOPPING:
                // Exit while engine is already stopping: just set exit code so engine will exit after stopping.
                get_service<engine_logger>().warning(
                    "Engine termination was requested while the engine is already stopping. "
                    "Engine will finish current stop procedure before exiting."
                );

                break;
            default:
                // Exit while the engine is running: stop normally.
                state = engine::STOP_REQUESTED;
                if (immediate) engine_exit();
        }
    }


    void engine::engine_init(void) {
        game_callbacks::pre_init();
        set_state(engine::INITIALIZING);

        // TODO: Init here!

        set_state(engine::RUNNING);
        game_callbacks::post_init();
    }


    void engine::engine_loop(void) {
        game_callbacks::pre_loop();

        // TODO: Loop here!

        game_callbacks::post_loop();
    }


    void engine::engine_exit(void) {
        if (state == engine::UNINITIALIZED_STOP_REQUESTED) {
            get_service<engine_logger>().warning(
                "Engine termination requested while the engine was still uninitialized. "
                "Normal exit procedure will be skipped and the engine will exit immediately."
            );

            goto exit_now;
        }


        game_callbacks::pre_exit();
        set_state(engine::STOPPING);

        set_state(engine::STOPPED);
        game_callbacks::post_exit();


        exit_now:
        if (exit_code) std::exit(*exit_code);
    }


    void engine::set_state(engine::engine_state state) {
        this->state = state;
        get_service<engine_logger>().info("Engine state changed to {}.", magic_enum::enum_name(state));
    }
}