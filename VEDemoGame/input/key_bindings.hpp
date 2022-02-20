#pragma once

#include <VoxelEngine/input/input.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


namespace demo_game {
    static inline ve::input_binder controls { };


    static inline ve::meta::null_type init_controls = [] {
        using kb_args = typename ve::keyboard_input::args;
        using mb_args = typename ve::mouse_button_input::args;


        // Motion & Look Controls
        controls.create_alias("move_forwards",  ve::keyboard_input { SDLK_w });
        controls.create_alias("move_backwards", ve::keyboard_input { SDLK_s });
        controls.create_alias("move_left",      ve::keyboard_input { SDLK_a });
        controls.create_alias("move_right",     ve::keyboard_input { SDLK_d });
        controls.create_alias("move_up",        ve::keyboard_input { SDLK_SPACE  });
        controls.create_alias("move_down",      ve::keyboard_input { SDLK_LSHIFT });
        controls.create_alias("look",           ve::mouse_motion_input { });


        // Mouse Capture Controls
        controls.create_alias(
            "capture_mouse",
            ve::mouse_button_input { mb_args {
                .button                 = ve::mouse_button::LEFT,
                .trigger_on             = ve::binary_input::KEY_DOWN,
                .requires_mouse_capture = false
            }}
        );

        controls.create_alias(
            "release_mouse",
            ve::keyboard_input { kb_args {
                .key        = SDLK_ESCAPE,
                .trigger_on = ve::binary_input::KEY_DOWN
            }}
        );


        return ve::meta::null_type { };
    } ();
}