#pragma once

#include <VoxelEngine/core/core.hpp>

#include <SDL_video.h>
#include <SDL_opengl.h>


namespace ve::gfx::opengl::window_helpers {
    constexpr inline std::underlying_type_t<SDL_WindowFlags> get_window_flags(void) {
        return SDL_WINDOW_OPENGL;
    }


    inline vec2ui get_canvas_size(SDL_Window* window) {
        i32 w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);

        return vec2ui { (u32) w, (u32) h };
    }


    constexpr inline auto get_supported_present_modes(void) {
        // OpenGL does not support triple buffering directly.
        return std::array {
            present_mode::IMMEDIATE,
            present_mode::VSYNC
        };
    }


     static bool is_present_mode_supported(present_mode_t mode) {
        return ranges::contains(get_supported_present_modes(), mode);
    }
}