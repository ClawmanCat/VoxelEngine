#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/input/binder/input_binder.hpp>
#include <VoxelEngine/input/input_manager.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>

// *****************************************************************
// * PLEASE READ BELOW MESSAGE BEFORE RUNNING THIS TEST OUTSIDE CI *
// *****************************************************************
//
// This test dispatches fake mouse input events to check the functionality of the input binder.
// Moving the mouse during this test will cause the test to fail due to extra events being generated.

test_result test_main(void) {
    // Required for dispatching fake events.
    auto window = ve::gfx::window::create(ve::gfx::window::arguments { .title = "Test Window" });


    using motion_t = ve::motion_input::key_t::motion_type_t;
    using when_t   = ve::motion_input::trigger_when_t;


    ve::input_binder binder;
    test_result result = VE_TEST_SUCCESS;


    ve::vec2i mouse_delta;
    bool started = false, ended = false;

    binder.bind(
        ve::motion_input { .input = { motion_t::MOUSE_DRAG, ve::mouse_button::LEFT }, .trigger_when = when_t::MOTION_START | when_t::MOTION_TICK | when_t::MOTION_END },
        [&] (const auto& args) {
            if (args.when == when_t::MOTION_START) {
                if (std::exchange(started, true)) result |= VE_TEST_FAIL("Motion start event triggered twice.");
            }

            else if (args.when == when_t::MOTION_END) {
                if (!started) result |= VE_TEST_FAIL("Motion end event triggered before start event.");
                if (std::exchange(ended, true)) result |= VE_TEST_FAIL("Motion end event triggered twice.");

                mouse_delta = args.current.position - args.begin.position;
            }

            else {
                if (!started) result |= VE_TEST_FAIL("Motion tick event triggered before start event.");
                if (ended) result |= VE_TEST_FAIL("Motion tick event triggered after end event.");
            }
        }
    );


    ve::vec2i per_tick_motion { 10, 10 };
    ve::u32 num_ticks = 10;

    SDL_Event motion_event;
    motion_event.type = SDL_MOUSEMOTION;
    motion_event.motion.windowID = SDL_GetWindowID(window->get_handle());
    motion_event.motion.xrel = per_tick_motion.x;
    motion_event.motion.yrel = per_tick_motion.y;

    SDL_Event button_toggle_event;
    button_toggle_event.button.windowID = SDL_GetWindowID(window->get_handle());
    button_toggle_event.button.button = SDL_BUTTON_LEFT;


    // The created window MUST be the focus, as the focussed window will be checked when the mouse is released
    // to dispatch the motion ended_event.
    SDL_WarpMouseInWindow(window->get_handle(), 0, 0);
    SDL_FlushEvent(SDL_MOUSEMOTION);

    auto could_focus = SDL_SetWindowInputFocus(window->get_handle());
    VE_ASSERT(could_focus, "Cannot perform test: failed to focus test window.");


    button_toggle_event.type = SDL_MOUSEBUTTONDOWN;
    SDL_PushEvent(&button_toggle_event);

    for (ve::u32 i = 0; i < num_ticks; ++i) {
        SDL_PushEvent(&motion_event);
        ve::input_manager::instance().update(i);
    }

    button_toggle_event.type = SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&button_toggle_event);

    ve::input_manager::instance().update(num_ticks);


    if (!started || !ended) result |= VE_TEST_FAIL("Not all handlers were called.");
    if (mouse_delta != per_tick_motion * (ve::i32) num_ticks) result |= VE_TEST_FAIL("Amount of motion indicated in sent events different from observed motion.");


    // Move the mouse again without dragging to check no events are dispatched.
    ve::vec2i mouse_delta_after_drag = mouse_delta;

    for (ve::u32 i = 0; i < num_ticks; ++i) {
        SDL_PushEvent(&motion_event);
        ve::input_manager::instance().update(i);
    }

    ve::input_manager::instance().update(num_ticks);


    if (mouse_delta_after_drag != mouse_delta) result |= VE_TEST_FAIL("Drag event handlers called for non-dragging motion.");


    return result;
}