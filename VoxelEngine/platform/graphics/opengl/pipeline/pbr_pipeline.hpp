#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline_builder.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/utility.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/quad.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/then.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace ve::gfx::opengl {
    class pbr_pipeline : public pipeline, public std::enable_shared_from_this<pbr_pipeline> {
    public:
        constexpr static inline u32 GAUSSIAN_HORIZONTAL = 0;
        constexpr static inline u32 GAUSSIAN_VERTICAL   = 1;


        ve_shared_only_then(pbr_pipeline, setup_pipeline, shared<render_target> target) :
            pipeline(&pipeline_category::RASTERIZATION, std::move(target))
        {}


        void draw(const draw_data& data) override {
            VE_PROFILE_FN();
            assert_has_render_mixins(*data.ctx, { "ve.lighting_mixin", "ve.bloom_mixin" });


            if (!get_target()->requires_rendering_this_frame()) return;


            raii_tasks on_destruct;

            data.ctx->pipelines.push(this);
            on_destruct.add_task([&] { data.ctx->pipelines.pop(); });
            data.ctx->uniform_state.push_uniforms(this);
            on_destruct.add_task([&] { data.ctx->uniform_state.pop_uniforms(); });


            // Note: the target of the postprocessing pass is the final target, so don't clear that, in case other stuff is rendered to it as well.
            geometry_pass->get_target()->clear();
            lighting_pass->get_target()->clear();


            // Same as draw data, but with the screen-filling quad as its only vertex buffer.
            draw_data quad_data = draw_data_to_g_input(data);

            geometry_pass->draw(data);
            lighting_pass->draw(quad_data);

            ping_pong_render(
                std::vector<shared<pipeline>> { bloom_blur_passes[0], bloom_blur_passes[1] },
                quad_data,
                bloom_passes,
                name_transforms::rename_one { .old_name = "bloom", .new_name = "tex" }
            );

            postprocessing_pass->draw(quad_data);
        }
    private:
        void setup_pipeline(void) {
            VE_PROFILE_FN();


            auto get_shader = [] <typename V> (meta::type_wrapper<V> vertex, const auto& name, const auto&... stages) {
                return shader_cache::instance().template get_or_load<V>(
                    std::vector<fs::path>{ io::paths::PATH_SHADERS / stages... },
                    name
                );
            };

            auto geometry_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::material_vertex_3d>{},
                "pbr_geometry",
                "pbr.vert.glsl", "pbr_geometry.frag.glsl"
            );

            auto lighting_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::no_vertex>{},
                "pbr_lighting",
                "screen_quad.g_input.glsl", "pbr_lighting.frag.glsl"
            );

            auto bloom_blur_shader = get_shader(
                meta::type_wrapper<vertex_types::no_vertex>{},
                "gaussian",
                "screen_quad.g_input.glsl", "gaussian.frag.glsl"
            );

            auto postprocessing_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::no_vertex>{},
                "pbr_postprocessing",
                "screen_quad.g_input.glsl", "pbr_postprocessing.frag.glsl"
            );


            auto renderpasses = build_pipeline(
                {
                    renderpass_definition { geometry_pass_shader }
                        | then([] (auto& pass) {
                            pass.add_depth_attachment("depth");
                            pass.add_color_attachments({ "position", "normal", "color", "material" });
                            pass.modify_attachment("position", [] (auto& a) { a.clear_value = vec4f { 0, 0, 0, 1 }; }); // position.z = depth
                        }),

                    renderpass_definition { lighting_pass_shader, name_transforms::add_prefix { "g_" } }
                        | then([] (auto& pass) {
                            pass.add_color_attachments({ "position", "color", "bloom" });
                            pass.modify_attachment("position", [] (auto& a) { a.clear_value = vec4f { 0, 0, 0, 1 }; }); // position.z = depth
                            pass.modify_attachment("bloom", [] (auto& a) { a.tex_filter = texture_filter::LINEAR; });
                        }),

                    renderpass_definition { bloom_blur_shader, name_transforms::rename_one { "bloom", "tex" } }
                        | then([] (auto& pass) {
                            pass.add_indirect_input(1, "position");
                            pass.add_color_attachment("bloom");
                            pass.modify_attachment("bloom", [] (auto& a) { a.tex_filter = texture_filter::LINEAR; });
                            pass.size_transform = size_transforms::rescale { 0.33 };
                        }),

                    renderpass_definition { bloom_blur_shader, name_transforms::rename_one { "bloom", "tex" } }
                        | then([] (auto& pass) {
                            pass.add_indirect_input(1, "position");
                            pass.add_color_attachment("bloom");
                            pass.modify_attachment("bloom", [] (auto& a) { a.tex_filter = texture_filter::LINEAR; });
                            pass.size_transform = size_transforms::rescale { 0.33 };
                        }),

                    renderpass_definition { postprocessing_pass_shader, name_transforms::add_prefix { "l_" } }
                        | then([] (auto& pass) {
                            pass.add_indirect_input(1, "position");
                            pass.add_indirect_input(1, "color");
                        })
                },
                get_target()
            );

            move_into(std::move(renderpasses), geometry_pass, lighting_pass, bloom_blur_passes[0], bloom_blur_passes[1], postprocessing_pass);


            bloom_blur_passes[0]->template set_uniform_value<u32>("direction", GAUSSIAN_HORIZONTAL);
            bloom_blur_passes[1]->template set_uniform_value<u32>("direction", GAUSSIAN_VERTICAL);
        }


        u32 bloom_passes = 4;

        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> bloom_blur_passes[2];
        shared<single_pass_pipeline> postprocessing_pass;
    public:
        VE_GET_SET_VAL(bloom_passes);
    };
}