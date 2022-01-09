#include <VEDemoGame/game.hpp>

#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/clientserver/connect.hpp>


namespace demo_game {
    using namespace ve;
    using namespace ve::gfx;
    using namespace ve::gfxapi;

    using simple_vertex = vertex_types::texture_vertex_3d;
    using pbr_vertex    = vertex_types::material_vertex_3d;


    void game::pre_init(void)  {}
    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}


    const game_info* game::get_info(void) {
        const static game_info info {
            .display_name = "Demo Game",
            .description  = { "Demonstrator to show engine functionality." },
            .authors      = { "ClawmanCat" },
            .version      = version {
                VEDEMOGAME_VERSION_MAJOR,
                VEDEMOGAME_VERSION_MINOR,
                VEDEMOGAME_VERSION_PATCH,
                "PreAlpha"
            }
        };

        return &info;
    }


    void game::post_init(void) {
        if (engine::get_arguments().has("server")) setup_server();
        if (engine::get_arguments().has("client")) setup_client();
    }


    void game::setup_client(void) {
        game::client = make_unique<class client>();


        // Create window & set up graphics pipelines.


        // Set up user input.


        // Perform local transform updates in case we don't receive anything from the server.


        // Set up remote initializers for entity classes.


        // Connect to server.
        auto remote_address = engine::get_arguments().value_or<std::string>("remote_address", "127.0.0.1");
        connect_remote(*game::client, remote_address, 6969);
    }


    void game::setup_server(void) {
        game::server = make_unique<class server>();


        // Set up server-side systems.


        // Synchronize entities with clients.


        // Allow clients to update their own position and velocity.


        // Create player entity when player connects, destroy entity when player disconnects.


        // Create entities.


        // Begin accepting connections from clients.
        host_server(*game::server, 6969);
    }
}