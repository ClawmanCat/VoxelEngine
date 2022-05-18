#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/uniform/uniform_storage.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/settings.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/target.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline_events.hpp>


namespace ve::renderer_mixins {
    class render_mixin_base;
}


namespace ve::gfx::opengl {
    template <typename Derived, typename Pipeline> class pipeline_mixin_base;

    class vertex_buffer;
    struct render_context;


    struct pipeline_draw_data {
        std::vector<const vertex_buffer*> buffers;
        render_context* ctx;
    };


    class pipeline : public uniform_storage, public subscribe_only_view<simple_event_dispatcher<>> {
    public:
        explicit pipeline(const pipeline_category_t* type, shared<render_target> target) :
            type(type), target(std::move(target))
        {}

        virtual ~pipeline(void) = default;


        // While this method is overridable, the method you want to override is probably the protected 2-argument one.
        virtual void draw(const pipeline_draw_data& data) {
            VE_PROFILE_FN();


            // Skip drawing if the texture the target has is still up-to-date.
            if (!target->requires_rendering_this_frame()) return;


            // Push pipeline & uniforms to the context stack while the pipeline is drawing.
            raii_tasks on_destruct;

            data.ctx->pipelines.push(this);
            on_destruct.add_task([&] { data.ctx->pipelines.pop(); });
            data.ctx->uniform_state.push_uniforms(this);
            on_destruct.add_task([&] { data.ctx->uniform_state.pop_uniforms(); });


            // Dispatch events and actually call the derived draw method.
            dispatch_event(pipeline_pre_draw_event { .pipeline = this, .draw_data = &data });
            draw(data, overridable_function_tag { });
            dispatch_event(pipeline_post_draw_event { .pipeline = this, .draw_data = &data });
        }
    protected:
        template <typename Derived, typename Pipeline> friend class pipeline_mixin_base;


        // Overridable overload of draw. Override this method to add actual rendering functionality to the pipeline.
        virtual void draw(const pipeline_draw_data& data, overridable_function_tag) = 0;

        // Asserts that the system_renderer that owns this pipeline has the given mixins.
        // If this pipeline is not part of a system_renderer, this check will fail for any mixin.
        // It is possible to disable these checks by calling set_mixin_checks_enabled(false),
        // in which case the user is responsible for making sure all the requirements of the pipeline are met.
        void assert_has_ecs_mixins(const render_context& ctx, const std::vector<std::string_view>& mixins) const;

        // Retrieves the given mixin from the system_renderer owning this pipeline.
        // If this pipeline is not part of a system_renderer or the system does not have the given mixin, this function returns nullptr.
        template <typename Mixin> const Mixin* get_ecs_mixin(const render_context& ctx, std::string_view name) {
            return (const Mixin*) get_ecs_mixin_impl(ctx, name);
        }

    private:
        const renderer_mixins::render_mixin_base* get_ecs_mixin_impl(const render_context& ctx, std::string_view name);


        const pipeline_category_t* type;
        shared<render_target> target;
        bool mixin_checks_enabled = false;

    public:
        VE_GET_VAL(type);
        VE_GET_SET_CREF(target);
        VE_GET_SET_VAL(mixin_checks_enabled);
    };


    class single_pass_pipeline : public pipeline {
    public:
        using single_pass_pipeline_tag = void;
        using pipeline::draw;


        single_pass_pipeline(shared<render_target> target, shared<class shader> shader, pipeline_settings settings = {})
            : pipeline(shader->get_category(), std::move(target)), settings(std::move(settings)), shader(std::move(shader))
        {}

        void draw(const pipeline_draw_data& data, overridable_function_tag) override;
        void set_shader(shared<class shader> shader);

        VE_GET_MREF(settings);
        VE_GET_CREF(shader);
    private:
        pipeline_settings settings;
        shared<class shader> shader;

        void bind_settings(void);
    };
}