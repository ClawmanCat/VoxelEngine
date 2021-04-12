#include <VEDemoGame/game.hpp>

#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/input/input_event.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    ve::shared<gfx::window> game::window = nullptr;
    ve::scene<ve::side::CLIENT> game::scene { };
    gfx::perspective_camera game::camera { };
    
    
    void game::on_pre_init(void) {}
    
    
    void game::on_post_init(void) {
        game::window = std::make_shared<gfx::window>(
            gfx::window::arguments { .title = get_info()->name.c_str() }
        );
        
        // Exit if all windows are closed.
        ve_register_input_event(ve::last_window_closed_event, _, ve::engine::request_exit());
        
        
        auto pipeline = std::make_shared<gfx::simple_pipeline>();
        window->add_layer(gfx::layerstack_target { pipeline }, 0.0f);
        
        game::camera.set_position(ve::vec3f { 0, 3, 0 });
        game::camera.set_fov(ve::radians(70.0f));
        
        ve_register_input_event(
            ve::window_resize_event,
            e,
            game::camera.set_aspect_ratio(e.new_size.x / e.new_size.y)
        );
        
        pipeline->set_uniform_producer<ve::mat4f>("camera"s, []() {
            return game::camera.get_matrix();
        });
    
        pipeline->set_uniform_producer<float>("near"s, []() {
            return game::camera.get_near();
        });
        
        game::scene.add_system(ve::renderer(pipeline));
        game::scene.add_system(ve::movement());
        game::scene.add_system(ve::static_updater<ve::side::SERVER>());
        
        game::scene.create_entity<world>();
        
        for (ve::i32 x = -32; x <= 32; x += 16) {
            for (ve::i32 z = -32; z <= 32; z += 16) {
                auto& h = game::scene.create_entity<howlee>();
                h.transform.position = ve::vec3f { x, 1, z };
            }
        }
    }
    
    
    void game::on_pre_loop(ve::u64 tick, microseconds dt) {
        auto& im = ve::input_manager::instance();
        
        
        // Capture mouse.
        static bool captured = false;
        if (im.is_pressed(ve::mouse_button::LEFT)) captured = true;
        if (im.is_pressed(SDLK_ESCAPE)) captured = false;
        
        im.set_mouse_capture(captured);
        if (!captured) return;
        
    
        float move_speed = float(dt.count()) / 50'000.0f;
        float look_speed = float(dt.count()) / 500'000.0f;
    
        
        // Rotate camera based on mouse input.
        ve::vec3f rotation = camera.get_rotation() + ve::vec3f {
            -im.tick_mouse_delta().y * look_speed,
            -im.tick_mouse_delta().x * look_speed,
            0.0f
        };
    
        const float max_rotation = 0.999f * (ve::pi_f / 2.0f);
        rotation.x = std::clamp(rotation.x, -max_rotation, max_rotation);
        
        camera.set_rotation(rotation);
    
    
        // Move camera based on keyboard input.
        float dx = (-move_speed * im.is_pressed(SDLK_a))      + (+move_speed * im.is_pressed(SDLK_d));
        float dy = (-move_speed * im.is_pressed(SDLK_LSHIFT)) + (+move_speed * im.is_pressed(SDLK_SPACE));
        float dz = (-move_speed * im.is_pressed(SDLK_s))      + (+move_speed * im.is_pressed(SDLK_w));
        
        game::camera.move(
            game::camera.get_right()                           * dx +
            ve::vec3f(ve::direction_vector(ve::direction::UP)) * dy +
            game::camera.get_forwards()                        * dz
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