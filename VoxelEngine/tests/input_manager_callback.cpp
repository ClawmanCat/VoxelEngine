#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


test_result test_main(void) {
    // Required for dispatching fake events.
    auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test Window" });

    std::array<SDL_Keycode, 2> keys = { SDLK_x, SDLK_y };
    std::array<bool, 2> pressed     = { false, false };
    std::array<bool, 2> down        = { false, false };

    test_result result = VE_TEST_SUCCESS;


    for (auto [i, key] : keys | ve::views::enumerate) {
        ve::input_manager::instance().add_handler([&, i = i] (const ve::key_down_event& e) {
            if (e.new_state.key != keys[i]) return;
            if (std::exchange(down[i], true) || pressed[i]) result |= VE_TEST_FAIL("Key down handler was called twice.");
        });

        ve::input_manager::instance().add_handler([&, i = i] (const ve::key_up_event& e) {
            if (e.new_state.key != keys[i]) return;
            if (!std::exchange(down[i], false))  result |= VE_TEST_FAIL("Key up handler was called before key down handler.");
            if (std::exchange(pressed[i], true)) result |= VE_TEST_FAIL("Key up handler was called twice.");
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