#include <VEDemoGame/game.hpp>
#include <VEDemoGame/entity/howlee.hpp>
#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/entity/world.hpp>
#include <VEDemoGame/entity/skybox.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/input/controls.hpp>

#include <VoxelEngine/clientserver/connect.hpp>
#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    using simple_vertex_t = ve::gfx::vertex_types::texture_vertex_3d;
    using pbr_vertex_t    = ve::gfx::vertex_types::material_vertex_3d;


    void game::pre_loop(void)  {}
    void game::post_loop(void) {}
    void game::pre_exit(void)  {}
    void game::post_exit(void) {}
    void game::pre_init(void)  {}

    
    void game::post_init(void) {
        if (ve::engine::get_arguments().has("run_client")) setup_client();
        if (ve::engine::get_arguments().has("run_server")) setup_server();
    }
    
    
    const ve::game_info* game::get_info(void) {
        const static ve::game_info info {
            .display_name = "Demo Game",
            .description  = { "Demonstrator to show engine functionality." },
            .authors      = { "ClawmanCat" },
            .version      = ve::version {
                VEDEMOGAME_VERSION_MAJOR,
                VEDEMOGAME_VERSION_MINOR,
                VEDEMOGAME_VERSION_PATCH,
                "PreAlpha"
            }
        };
    
        return &info;
    }



    void game::setup_client(void) {
        // Set up global graphics objects.
        game::window = ve::gfx::window::create(ve::gfx::window::arguments {
            .title = game::get_info()->display_name
        });

        ve::input_manager::instance().add_handler([] (const ve::window_resized_event& e) {
            if (e.window == game::window.get()) {
                auto size = ve::vec2f { e.new_size };
                game::camera.set_aspect_ratio(size.x / size.y);
            }
        });


        game::texture_manager = ve::make_shared<ve::gfx::texture_manager<>>();


        // Simple pipeline for rendering objects without lighting.
        auto simple_pipeline = make_shared<ve::gfxapi::single_pass_pipeline>(
            game::window->get_canvas(),
            ve::gfx::shader_cache::instance().get_or_load_shader<simple_vertex_t>("simple")
        );

        simple_pipeline->set_uniform_producer(&game::camera);
        simple_pipeline->set_uniform_producer(game::texture_manager->get_atlas());


        // Pipeline for rendering objects with PBR materials.
        auto pbr_pipeline = ve::gfxapi::multipass_pbr_pipeline::create(game::window->get_canvas());

        pbr_pipeline->set_uniform_producer(&game::camera);
        pbr_pipeline->set_uniform_producer(game::texture_manager->get_atlas());


        // Set up pipelines.
        using component_uniforms = ve::meta::pack<ve::transform_component>;
        using simple_renderer    = ve::meta::pack<render_tag_simple>;
        using pbr_renderer       = ve::meta::pack<render_tag_pbr>;


        auto [sr_id,  sr_system ] = game::client.add_system(ve::system_renderer<component_uniforms, simple_renderer> { simple_pipeline });
        auto [pbr_id, pbr_system] = game::client.add_system(ve::system_renderer<component_uniforms, pbr_renderer> { pbr_pipeline });

        pbr_system.get_lighting().ambient_light = ve::vec3f { 1.0f, 1.0f, 0.98f };

        game::client.add_system(ve::system_bind_camera<decltype(game::camera)> { });


        // Set up non-player-control hotkeys.
        controls.bind("capture_mouse", [](const ve::binary_input::handler_args& args) { ve::input_manager::instance().set_mouse_capture(true);  });
        controls.bind("release_mouse", [](const ve::binary_input::handler_args& args) { ve::input_manager::instance().set_mouse_capture(false); });
        ve::input_manager::instance().set_mouse_capture(true);


        // Set up synchronization.
        auto [vis_id, vis_system] = game::client.add_system(ve::system_set_visibility {});
        auto [ps_id,  ps_system ] = game::client.add_system(ve::system_remote_init {});

        // For entities with the player controlled tag, synchronize the transform from client to server.
        game::client.add_system(ve::system_synchronizer<
            ve::meta::pack<ve::transform_component>,
            ve::meta::pack<player_controlled_tag>
        > { &vis_system });

        ps_system.add_initializer<typename howlee::remote_init_tag>(&howlee::remote_initializer);
        ps_system.add_initializer<typename ve::synchronizable_voxel_component::remote_init_tag>(&ve::synchronizable_voxel_component::remote_initializer);
    }


    void game::setup_server(void) {
        // Setup basic server systems.
        game::server.add_system(ve::system_updater<> { });
        game::server.add_system(ve::system_physics<> { });


        // Setup synchronization.
        auto [vis_id, vis_system] = game::client.add_system(ve::system_set_visibility {});
        auto [ps_id,  ps_system ] = game::client.add_system(ve::system_remote_init {});

        using synced_components = ve::meta::pack<
            ve::transform_component,
            ve::light_component
        >;

        game::client.add_system(ve::system_synchronizer<synced_components> { &vis_system });


        // Create entities.
        auto& entity_player = game::server.store_static_entity(player { game::server, &game::camera });
        auto& entity_world  = game::server.store_static_entity(world  { game::server });

        entity_world.space->add_chunk_loader(ve::make_shared<ve::voxel::entity_loader<>>(entity_player.get_id(), game::server, ve::voxel::tilepos { 5 }));


        for (ve::i32 x = -32; x < 32; ++x) {
            for (ve::i32 z = -32; z < 32; ++z) {
                // Don't spawn the Howlees in debug mode to increase performance.
                VE_RELEASE_ONLY(
                    howlee& h = game::server.store_static_entity(howlee { game::server, &entity_world });
                    h.transform.position = ve::vec3f { x, 50.0f, z };
                );
            }
        }
    }
}