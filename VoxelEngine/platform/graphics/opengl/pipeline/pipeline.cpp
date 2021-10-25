#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::gfx::opengl {
    void single_pass_pipeline::draw(const draw_data& data) {
        if (!get_target()->requires_rendering_this_frame()) return;

        data.ctx->pipelines.push(this);
        data.ctx->renderpass = this;

        shader->bind(*data.ctx);
        get_target()->bind();
        bind_settings();


        if (shader->has_uniform(data.lighting_target)) {
            // For single-pass shading, treat lights as uniforms.
            // TODO: Handle variable number of lights better.
            lighting_data<> lights {
                .num_populated_lights = (u32) data.lights.size(),
                .ambient_light = data.ambient_light
            };

            VE_ASSERT(decltype(lights)::light_count_limit >= data.lights.size(), "Maximum number of lights exceeded.");
            std::copy(data.lights.begin(), data.lights.end(), lights.lights.begin());

            uniform_storage::set_uniform_value<lighting_data<>>(data.lighting_target, lights);
        }


        data.ctx->uniform_state.push_uniforms(this);
        for (const auto& buffer : data.buffers) buffer->draw(*data.ctx);
        data.ctx->uniform_state.pop_uniforms();


        data.ctx->renderpass = nullptr;
        data.ctx->pipelines.pop();
    }


    void single_pass_pipeline::bind_settings(void) {
        std::array gl_toggles { glDisable, glEnable };


        // Note: OpenGL handles the primitive topology on a per-drawcall basis,
        // so that setting does not need to be set here.
        glPolygonMode(GL_FRONT_AND_BACK, (GLenum) settings.polygon_mode);
        glFrontFace((GLenum) settings.cull_direction);
        glLineWidth(settings.line_width);

        auto no_cull = pipeline_settings::cull_mode_t::NO_CULLING;
        gl_toggles[settings.cull_mode != no_cull](GL_CULL_FACE);
        if (settings.cull_mode != no_cull) glCullFace((GLenum) settings.cull_mode);

        gl_toggles[settings.depth_testing](GL_DEPTH_TEST);
        gl_toggles[settings.depth_clamp](GL_DEPTH_CLAMP);
    }
}