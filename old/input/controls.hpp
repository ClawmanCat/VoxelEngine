#pragma once

#include <VoxelEngine/input/input_binder.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


namespace demo_game {
    static inline ve::input_binder controls { };


    static inline ve::meta::null_type initialize_controls = [] {
        using motion_t = ve::motion_input::key_t::motion_type_t;

        using bi_when = ve::binary_input::trigger_when_t;
        using mi_when = ve::motion_input::trigger_when_t;

        controls.alias("move_forwards",  ve::binary_input { .input = SDLK_w,      .trigger_when = bi_when::KEY_HOLD });
        controls.alias("move_backwards", ve::binary_input { .input = SDLK_s,      .trigger_when = bi_when::KEY_HOLD });
        controls.alias("move_left",      ve::binary_input { .input = SDLK_a,      .trigger_when = bi_when::KEY_HOLD });
        controls.alias("move_right",     ve::binary_input { .input = SDLK_d,      .trigger_when = bi_when::KEY_HOLD });
        controls.alias("move_up",        ve::binary_input { .input = SDLK_SPACE,  .trigger_when = bi_when::KEY_HOLD });
        controls.alias("move_down",      ve::binary_input { .input = SDLK_LSHIFT, .trigger_when = bi_when::KEY_HOLD });
        controls.alias("look",           ve::motion_input { .input = { motion_t::MOUSE_MOVE }, .trigger_when = mi_when::MOTION_TICK });


        controls.alias("capture_mouse", ve::binary_input { .input = ve::mouse_button::LEFT, .trigger_when = bi_when::KEY_DOWN, .requires_mouse_capture = false });
        controls.alias("release_mouse", ve::binary_input { .input = SDLK_ESCAPE, .trigger_when = bi_when::KEY_DOWN });


        return ve::meta::null_type { };
    }();
}