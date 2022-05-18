#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/lighting/gaussian_blur.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/utility.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/renderpass_transforms.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline_events.hpp>


// Provides mixins for pipelines to add optional rendering features.
namespace ve::gfx::opengl {
    template <typename Derived, typename Pipeline> class pipeline_mixin {
    public:
        void pre_draw(const Pipeline& self, const pipeline_draw_data& data) {
            VE_MAYBE_CRTP_CALL(Derived, pre_draw, self, data);
        }

        void post_draw(const Pipeline& self, const pipeline_draw_data& data) {
            VE_MAYBE_CRTP_CALL(Derived, post_draw, self, data);
        }

    protected:
        Pipeline& get_pipeline(void) { return *static_cast<Pipeline*>(this); }
        const Pipeline& get_pipeline(void) const { return *static_cast<Pipeline*>(this); }

        // Friend access.
        void assert_has_ecs_mixins(const render_context& ctx, const std::vector<std::string_view>& mixins) const {
            get_pipeline().assert_has_ecs_mixins(ctx, mixins);
        }

        // Friend access.
        template <typename Mixin> const Mixin* get_ecs_mixin(const render_context& ctx, std::string_view name) {
            return get_pipeline().template get_ecs_mixin<Mixin>(ctx, name);
        }
    };


    template <typename Pipeline>
    class pipeline_bloom_mixin : public pipeline_mixin<pipeline_bloom_mixin<Pipeline>, Pipeline> {
    public:
        void pre_draw(const Pipeline& self, const pipeline_draw_data& data) {
            static_assert(
                requires { typename Pipeline::pbr_pipeline_tag; },
                "The Bloom Pipeline Mixin can only be used with PBR Pipelines."
            );

            // Assert bloom data is provided.
            const static std::vector<std::string_view> required_mixins { "ve.bloom_mixin" };
            assert_has_ecs_mixins(*data.ctx, required_mixins);
        }


        void create_bloom_passes(shared<render_target> position, shared<render_target> color) {
            auto gaussian_blur_shader = get_shader<vertex_types::no_vertex>(
                "gaussian",
                "pipeline_common/screen_quad.g_input.glsl", "pipeline_common/gaussian.frag.glsl"
            );

            auto attachment_template = framebuffer_attachment_template { attachment_name };
            attachment_template.tex_filter = texture_filter::LINEAR;



            horizontal_pass->set_uniform_value<u32>("direction", GAUSSIAN_HORIZONTAL);
            vertical_pass->set_uniform_value<u32>("direction", GAUSSIAN_VERTICAL);


            // Add handler post lighting to exec bloom passes.
        }


        void execute_bloom_passes(const Pipeline& self, const pipeline_draw_data& data) {
            ping_pong_render(
                std::vector<shared<pipeline>> { horizontal_pass, vertical_pass },
                quad_data,
                bloom_passes,
                name_transforms::rename_one { .old_name = attachment_name, .new_name = sampler_name }
            );
        }

    private:
        u32 num_bloom_passes = 4;
        std::string attachment_name = "bloom", sampler_name = "tex";
        shared<single_pass_pipeline> horizontal_pass, vertical_pass;

        event_handler_id_t pre_pipeline_build, post_pipeline_build;

    public:
        VE_GET_SET_VAL(num_bloom_passes);
        VE_GET_SET_CREF(attachment_name);
        VE_GET_SET_CREF(sampler_name);
        VE_GET_VAL(horizontal_pass);
        VE_GET_VAL(vertical_pass);
    };
}