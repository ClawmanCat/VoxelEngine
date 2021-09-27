#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/presentation/window_helpers.hpp>

#include <SDL_video.h>


namespace ve::gfx::opengl {
    class canvas {
    public:
        canvas(SDL_Window* owner, present_mode_t mode) : owner(owner) {
            set_present_mode(mode);
        }


        void begin_frame(void) {
            make_current();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }


        void end_frame(void) {
            make_current();

            auto canvas_size = window_helpers::get_canvas_size(owner);
            glViewport(0, 0, (GLsizei) canvas_size.x, (GLsizei) canvas_size.y);

            // TODO: Blit framebuffer to canvas.

            SDL_GL_SwapWindow(owner);
        }


        void set_present_mode(present_mode_t mode) {
            VE_ASSERT(window_helpers::is_present_mode_supported(mode), "Unsupported present mode: ", mode.name);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, (mode.preferred_images > 1) ? SDL_TRUE : SDL_FALSE);
            SDL_GL_SetSwapInterval(mode == present_mode::VSYNC ? 1 : 0);

            this->mode = mode;
        }


        VE_GET_VAL(owner);
        VE_GET_VAL(mode);
    private:
        SDL_Window* owner;
        present_mode_t mode;


        void make_current(void) {
            SDL_GL_MakeCurrent(owner, get_context()->handle);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };
}