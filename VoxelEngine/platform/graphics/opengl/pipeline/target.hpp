#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/framebuffer.hpp>


namespace ve::gfx::opengl {
    class render_target : public framebuffer {
    public:
        render_target(void) = default;


        render_target(
            const std::vector<framebuffer_attachment_template>& templates,
            std::function<vec2ui(void)> texture_validator,
            std::function<bool(void)> render_validator
        ) :
            framebuffer(templates, std::move(texture_validator)),
            render_validator(std::move(render_validator))
        {}


        // Keep this as an explicit method rather than using the copy constructor to prevent accidental copies and to allow overriding.
        virtual shared<render_target> clone(void) const {
            return make_shared<render_target>(
                get_attachments() | views::values | views::transform(&framebuffer_attachment::attachment_template) | ranges::to<std::vector>,
                get_texture_validator(),
                render_validator
            );
        }


        bool requires_rendering_this_frame(void) const {
            return render_validator();
        }

    private:
        // The render validator returns if the target should be re-rendered before the next frame.
        std::function<bool(void)> render_validator;

    public:
        VE_GET_CREF(render_validator);
    };
}