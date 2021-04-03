#include <VEDemoGame/game.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    ve::simple_tile_storage& get_tile_store(void) {
        static ve::simple_tile_storage instance;
        return instance;
    }
    
    const ve::tile* tile_grass = get_tile_store().emplace<ve::tile>(ve::tile_parameters { .name = "grass" });
    const ve::tile* tile_stone = get_tile_store().emplace<ve::tile>(ve::tile_parameters { .name = "stone" });
    
    
    void game::on_pre_init(void) {}
    
    
    void game::on_post_init(void) {
        game::window = std::make_shared<gfx::window>(
            gfx::window::arguments { .title = get_info()->name.c_str() }
        );
        
        // Exit if all windows are closed.
        ve_register_input_event(ve::last_window_closed_event, _, ve::engine::request_exit());
        
        // Capture mouse.
        ve::input_manager::instance().set_mouse_capture(true);
        
        
        auto pipeline = std::make_shared<gfx::simple_pipeline>();
        window->add_layer(gfx::layerstack_target { pipeline }, 0.0f);
        
        game::camera.set_position(ve::vec3f { -1 });
        game::camera.set_fov(ve::radians(70.0f));
        
        pipeline->set_uniform_producer<ve::mat4f>("camera"s, []() {
            return game::camera.get_matrix();
        });
        
        game::scene.add_system(ve::renderer(pipeline));
        game::scene.create_entity<world>();
    }
    
    
    void game::on_pre_loop(ve::u64 tick, microseconds dt) {
        // VE_LOG_DEBUG("Tick took "s + std::to_string(dt.count()) + "us.");
        
        auto& im = ve::input_manager::instance();
    
        float move_speed = float(dt.count()) / 50'000.0f;
        float look_speed = float(dt.count()) / 500'000.0f;
    
        
        // Rotate camera based on mouse input.
        game::camera.rotate({
            -im.tick_mouse_delta().y * look_speed,
            -im.tick_mouse_delta().x * look_speed,
            0.0f
        });
    
    
        // Move camera based on keyboard input.
        float dx = (-move_speed * im.is_pressed(SDLK_a))      + (+move_speed * im.is_pressed(SDLK_d));
        float dy = (-move_speed * im.is_pressed(SDLK_LSHIFT)) + (+move_speed * im.is_pressed(SDLK_SPACE));
        float dz = (-move_speed * im.is_pressed(SDLK_s))      + (+move_speed * im.is_pressed(SDLK_w));
        
        game::camera.move(
            game::camera.get_right()    * dx +
            game::camera.get_up()       * dy +
            game::camera.get_forwards() * dz
        );
    }
    
    
    void game::on_post_loop(ve::u64 tick, microseconds dt) {}
    
    
    void game::on_pre_exit(void)  {}
    void game::on_post_exit(void) {}
    
    
    [[nodiscard]] const ve::game_info* game::get_info(void) {
        static const ve::game_info info {
            .name         = "VoxelEngine Demo Game",
            .description  = { "Example game to demonstrate basic functionality." },
            .authors      = { "ClawmanCat" },
            .game_version = {
                "PreAlpha",
                VEDEMOGAME_VERSION_MAJOR,
                VEDEMOGAME_VERSION_MINOR,
                VEDEMOGAME_VERSION_PATCH
            }
        };
        
        return &info;
    }
}