#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/framebuffer.hpp>


namespace ve::gfx::opengl {
    class render_target : public framebuffer {
    public:
        render_target(void) = default;


        render_target(
            const std::vector<framebuffer_attachment>& attachments,
            std::function<vec2ui(void)> texture_validator,
            std::function<bool(void)> render_validator
        ) :
            framebuffer(attachments, std::move(texture_validator)),
            render_validator(std::move(render_validator))
        {}


        bool requires_rendering_this_frame(void) const {
            return render_validator();
        }
    private:
        // The render validator returns if the target should be re-rendered before the next frame.
        std::function<bool(void)> render_validator;
    };
}