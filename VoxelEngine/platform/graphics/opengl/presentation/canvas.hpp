#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/presentation/window_helpers.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target.hpp>

#include <SDL_video.h>


namespace ve::gfx::opengl {
    class canvas : public render_target {
    public:
        canvas(SDL_Window* owner, present_mode_t mode) :
            render_target(
                std::vector { framebuffer_attachment::COLOR_BUFFER, framebuffer_attachment::DEPTH_BUFFER },
                [owner] { return window_helpers::get_canvas_size(owner); }, // Framebuffer size.
                produce(true) // Always re-render.
            ),
            owner(owner)
        {
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

            glBlitNamedFramebuffer(
                framebuffer::get_id(),
                0, // Window framebuffer
                0, 0, (GLint) canvas_size.x, (GLint) canvas_size.y,
                0, 0, (GLint) canvas_size.x, (GLint) canvas_size.y,
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST
            );

            SDL_GL_SwapWindow(owner);
        }


        void set_present_mode(present_mode_t mode) {
            VE_ASSERT(window_helpers::is_present_mode_supported(mode), "Unsupported present mode: ", mode.name);

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