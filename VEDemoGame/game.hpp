#pragma once

#include <VoxelEngine/dependent/game_callbacks.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <iostream>


namespace demo_game {
    class game {
    public:
        game(void) = delete;

        static void pre_init(void);
        static void post_init(void);

        static void pre_loop(void);
        static void post_loop(void);

        static void pre_exit(void);
        static void post_exit(void);

        static const ve::game_info* get_info(void);


        VE_GET_STATIC_CREF(client);
        VE_GET_STATIC_CREF(server);
        VE_GET_STATIC_VAL(window);
        VE_GET_STATIC_VAL(texture_manager);
        VE_GET_STATIC_MREF(camera);
        VE_GET_STATIC_MREF(client_logger);
        VE_GET_STATIC_MREF(server_logger);
    private:
        static inline ve::unique<ve::client> client = nullptr;
        static inline ve::unique<ve::server> server = nullptr;

        static inline ve::shared<ve::gfx::window> window = nullptr;
        static inline ve::shared<ve::gfx::texture_manager<>> texture_manager = nullptr;
        static inline ve::gfx::perspective_camera camera { };

        static inline auto log_file = std::ofstream(ve::io::paths::PATH_LOGS / "demogame.log");
        static inline ve::logger client_logger = { "Client", ve::logger::level::VE_IF_DEBUG_ELSE(DEBUG, INFO), std::cout, log_file };
        static inline ve::logger server_logger = { "Server", ve::logger::level::VE_IF_DEBUG_ELSE(DEBUG, INFO), std::cout, log_file };


        static void setup_client(void);
        static void setup_client_graphics(void);
        static void setup_client_input(void);
        static void setup_client_synchronization(void);
        static void setup_client_systems(void);
        static void setup_client_connection(void);

        static void setup_server(void);
        static void setup_server_synchronization(void);
        static void setup_server_systems(void);
        static void setup_server_entities(void);
    };
}


namespace ve::game_callbacks {
    using game = demo_game::game;


    inline void pre_init(void)  { game::pre_init();  }
    inline void post_init(void) { game::post_init(); }

    inline void pre_loop(void)  { game::pre_loop();  }
    inline void post_loop(void) { game::post_loop(); }

    inline void pre_exit(void)  { game::pre_exit();  }
    inline void post_exit(void) { game::post_exit(); }

    inline const game_info* get_info(void) { return game::get_info(); }
}