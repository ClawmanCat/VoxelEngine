#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


test_result test_main(void) {
    using kb_args = typename ve::keyboard_input::args;
    using trigger = typename ve::binary_input::trigger_on_t;


    // Required for dispatching fake events.
    auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test Window" });
    ve::input_manager::instance().set_mouse_capture(true);

    std::array<SDL_Keycode, 2> keys = { SDLK_x, SDLK_y };
    std::array<bool, 2> pressed     = { false, false };
    std::array<bool, 2> down        = { false, false };

    ve::input_binder binder;
    test_result result = VE_TEST_SUCCESS;


    for (auto [i, key] : keys | ve::views::enumerate) {
        std::string binding_name = std::string { "kh_" } + SDL_GetKeyName(key);

        binder.create_alias(
            binding_name,
            ve::keyboard_input { kb_args { .key = key, .trigger_on = trigger::KEY_DOWN | trigger::KEY_UP } }
        );

        binder.template add_specialized_binding<ve::input_categories::keyboard_events>(binding_name, [&, i = i] <typename E> (const E& args) {
            if constexpr (std::is_same_v<E, ve::key_down_event>) {
                if (down[i] || pressed[i]) result |= VE_TEST_FAIL("Key down handler was called twice.");
                down[i] = true;
            } else {
                if (!down[i] || pressed[i]) result |= VE_TEST_FAIL("Key up handler was called twice or before key down handler.");
                down[i] = false;
                pressed[i] = true;
            }
        });
    }


    SDL_Event event;
    event.key.windowID = SDL_GetWindowID(window->get_handle());

    for (const auto& key : keys) {
        event.key.keysym.sym = key;

        event.type = SDL_KEYDOWN;
        SDL_PushEvent(&event);

        event.type = SDL_KEYUP;
        SDL_PushEvent(&event);
    }


    ve::input_manager::instance().update(0);
    for (bool b : pressed) if (!b) result |= VE_TEST_FAIL("Not all handlers were called.");


    return result;
}