#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/framebuffer.hpp>


namespace ve::gfx::opengl {
    struct render_target_settings {
        vec4f clear_color = { 0.65f, 0.65f, 0.65f, 1.00f };
        float clear_depth = 1.0f;
    };


    class render_target : public framebuffer {
    public:
        render_target(void) = default;


        render_target(
            const std::vector<framebuffer_attachment>& attachments,
            std::function<vec2ui(void)> texture_validator,
            std::function<bool(void)> render_validator,
            const render_target_settings& settings = { }
        ) :
            framebuffer(attachments, std::move(texture_validator)),
            render_validator(std::move(render_validator)),
            settings(settings)
        {}


        // Keep this as an explicit method rather than using the copy constructor to prevent accidental copies and to allow overriding.
        virtual shared<render_target> clone(void) const {
            return make_shared<render_target>(
                get_attachment_templates(),
                get_texture_validator(),
                render_validator,
                settings
            );
        }


        bool requires_rendering_this_frame(void) const {
            return render_validator();
        }


        void clear(void) {
            bind();

            glClearColor(settings.clear_color.r, settings.clear_color.g, settings.clear_color.b, settings.clear_color.a);
            glClearDepth(settings.clear_depth);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

    private:
        // The render validator returns if the target should be re-rendered before the next frame.
        std::function<bool(void)> render_validator;
        render_target_settings settings;

    public:
        VE_GET_CREF(render_validator);
        VE_GET_SET_CREF(settings);
    };
}