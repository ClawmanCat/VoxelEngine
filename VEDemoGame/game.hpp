#pragma once

#include <VoxelEngine/dependent/game_callbacks.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/ecs/ecs.hpp>


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
    private:
        static inline ve::unique<ve::client> client = nullptr;
        static inline ve::unique<ve::server> server = nullptr;

        static inline ve::shared<ve::gfx::window> window = nullptr;
        static inline ve::shared<ve::gfx::texture_manager<>> texture_manager = nullptr;
        static inline ve::gfx::perspective_camera camera { };


        static void setup_client(void);
        static void setup_server(void);
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