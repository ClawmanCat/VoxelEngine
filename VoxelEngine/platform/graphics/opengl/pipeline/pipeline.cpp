#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/ecs/system/system_renderer_mixins.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::gfx::opengl {
    void pipeline::assert_has_render_mixins(const render_context& ctx, const std::vector<std::string_view>& mixins) const {
        if (!mixin_checks_enabled) return;


        auto assert_fail = [] (const auto& missing_mixin) {
            VE_ASSERT(
                false,
                "This pipeline requires ", missing_mixin, " as part of the renderer, but it was not present.\n",
                "Note: to use the pipeline without an associated renderer, use pipeline.set_mixin_checks_enabled(false) ",
                "to disable this check. In this case, you must provide all required uniform values manually."
            );
        };


        if (!mixins.empty()) {
            if (!ctx.has_object("ve.render_mixins")) assert_fail(mixins.front());

            const auto& mixin_list = ctx.template get_object<renderer_mixins::mixin_list_t>("ve.render_mixins");
            for (const auto& mixin : mixins) {
                if (!mixin_list.contains(mixin)) assert_fail(mixin);
            }
        }
    }


    void single_pass_pipeline::draw(const draw_data& data) {
        VE_PROFILE_FN();

        if (!get_target()->requires_rendering_this_frame()) return;


        raii_tasks on_destruct;

        data.ctx->pipelines.push(this);
        on_destruct.add_task([&] { data.ctx->pipelines.pop(); });
        data.ctx->renderpass = this;
        on_destruct.add_task([&] { data.ctx->renderpass = nullptr; });
        data.ctx->uniform_state.push_uniforms(this);
        on_destruct.add_task([&] { data.ctx->uniform_state.pop_uniforms(); });


        shader->bind(*data.ctx);
        get_target()->bind();
        bind_settings();


        for (const auto& buffer : data.buffers) buffer->draw(*data.ctx);
    }


    void single_pass_pipeline::bind_settings(void) {
        VE_PROFILE_FN();

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