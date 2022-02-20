#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/input/keyboard.hpp>
#include <VoxelEngine/input/mouse.hpp>


namespace ve::input_categories {
    // Events which indicate some input is either on or off.
    using binary_events = meta::pack<
        key_down_event, key_up_event, key_hold_event, key_type_event,
        button_press_event, button_release_event, button_hold_event
    >;

    // Events which indicate some input changed by a scalar amount.
    using motion_events_1d = meta::pack<
        mouse_wheel_moved_event, mouse_wheel_move_start_event, mouse_wheel_move_end_event
    >;

    // Same as above, but only events which contain an old and a new value to compare.
    using motion_progress_events_1d = meta::pack<
        mouse_wheel_moved_event, mouse_wheel_move_end_event
    >;

    // Events which indicate some input changed by a 2d vector amount.
    using motion_events_2d = meta::pack<
        mouse_moved_event, mouse_move_start_event, mouse_move_end_event,
        mouse_drag_event, mouse_drag_start_event, mouse_drag_end_event
    >;

    // Same as above, but only events which contain an old and a new value to compare.
    using motion_progress_events_2d = meta::pack<
        mouse_moved_event, mouse_move_end_event,
        mouse_drag_event, mouse_drag_end_event
    >;
}