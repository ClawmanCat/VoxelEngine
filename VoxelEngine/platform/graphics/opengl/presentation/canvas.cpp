#include <VoxelEngine/platform/graphics/opengl/presentation/canvas.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/utility/then.hpp>
#include <VoxelEngine/utility/color.hpp>


namespace ve::gfx::opengl {
    canvas::canvas(window* owner, present_mode_t mode) :
        render_target(
            std::vector {
                framebuffer_attachment_template { "color", framebuffer_attachment_template::COLOR_BUFFER }
                    | then([] (auto& att) { att.clear_value = normalize_color_alpha(colors::SKY_BLUE); }),
                framebuffer_attachment_template { "depth", framebuffer_attachment_template::DEPTH_BUFFER }
            },
            [owner] { return window_helpers::get_canvas_size(owner->get_handle()); },
            // The fact that targets are shared resources provides a lifetime issue:
            // there is no way to force users to give up their references once a window has closed.
            // We can however set the canvas to never re-render if the window is closed, and just print a warning when the canvas is bound.
            [owner] { return !owner->is_closed(); }
        ),
        owner(owner)
    {
        set_present_mode(mode);
    }


    void canvas::bind(void) {
        // TODO: This warning is currently not triggered from pipelines, since they check should_render_this_frame before binding.
        // They should probably still warn, since its probably not a good idea to even have a pipeline that has an invalid target to begin with.
        if (owner->is_closed() && !warned_on_closed_render) {
            VE_LOG_ERROR(
                "Attempt to bind render target for window that has already closed. "
                "Any attempts to render to this target will be ignored. "
            );

            warned_on_closed_render = true;
        }


        render_target::bind();
    }


    void canvas::begin_frame(void) {
        render_target::clear();
    }


    void canvas::end_frame(void) {
        make_current();

        auto canvas_size = window_helpers::get_canvas_size(owner->get_handle());
        glViewport(0, 0, (GLsizei) canvas_size.x, (GLsizei) canvas_size.y);

        auto input_size = get_attachments().at("color").texture->get_size();
        glBlitNamedFramebuffer(
            framebuffer::get_id(),
            0, // Window framebuffer
            0, 0, (GLint) input_size.x,  (GLint) input_size.y,
            0, 0, (GLint) canvas_size.x, (GLint) canvas_size.y,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        SDL_GL_SwapWindow(owner->get_handle());
    }


    void canvas::set_present_mode(present_mode_t mode) {
        VE_ASSERT(window_helpers::is_present_mode_supported(mode), "Unsupported present mode: ", mode.name);

        SDL_GL_SetSwapInterval(mode == present_mode::VSYNC ? 1 : 0);
        this->mode = mode;
    }


    void canvas::make_current(void) {
        SDL_GL_MakeCurrent(owner->get_handle(), get_context()->handle);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}