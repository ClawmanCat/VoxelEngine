#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>


namespace ve::gfx::opengl {
    void single_pass_pipeline::draw(const std::vector<const vertex_buffer*>& buffers, render_context& ctx) {
        if (!get_target()->requires_rendering_this_frame()) return;


        ctx.pipelines.push(this);
        ctx.renderpass = this;

        get_target()->bind();
        shader->bind(ctx);
        bind_settings();

        uniform_storage::push(ctx.uniform_state);


        for (const auto& buffer : buffers) {
            buffer->draw(ctx);
        }


        uniform_storage::pop(ctx.uniform_state);

        ctx.renderpass = nullptr;
        ctx.pipelines.pop();
    }


    void single_pass_pipeline::bind_settings(void) {
        // Note: OpenGL handles the primitive topology on a per-buffer basis,
        // so that setting does not need to be set here.
        glPolygonMode(GL_FRONT_AND_BACK, (GLenum) settings.polygon_mode);
        glCullFace((GLenum) settings.cull_mode);
        glFrontFace((GLenum) settings.cull_direction);
        glLineWidth(settings.line_width);

        std::array gl_toggles { glDisable, glEnable };
        gl_toggles[settings.depth_testing](GL_DEPTH_TEST);
        gl_toggles[settings.depth_clamp](GL_DEPTH_CLAMP);
    }
}