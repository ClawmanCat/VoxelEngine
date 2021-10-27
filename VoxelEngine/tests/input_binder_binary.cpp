#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/input/input_binder.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


test_result test_main(void) {
    // Required for dispatching fake events.
    auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test Window" });


    using when_t = ve::binary_input::trigger_when_t;


    std::array<SDL_Keycode, 2> keys = { SDLK_x, SDLK_y };
    std::array<bool, 2> pressed     = { false, false };
    std::array<bool, 2> down        = { false, false };

    ve::input_binder binder;
    test_result result = VE_TEST_SUCCESS;


    for (auto [i, key] : keys | ve::views::enumerate) {
        binder.bind(ve::binary_input { .input = key, .trigger_when = when_t::KEY_DOWN | when_t::KEY_UP }, [&, i = i](auto mods, auto when) {
            if (when == when_t::KEY_DOWN) {
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