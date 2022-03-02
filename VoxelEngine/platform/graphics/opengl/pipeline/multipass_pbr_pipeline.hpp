#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/chainer.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/quad.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/raii.hpp>


namespace ve::gfx::opengl {
    // TODO: Migrate to generic class for multipass pipelines.
    class multipass_pbr_pipeline : public pipeline, public std::enable_shared_from_this<multipass_pbr_pipeline> {
    public:
        ve_shared_only_then(multipass_pbr_pipeline, setup_pipeline, shared<render_target> target) :
            pipeline(&pipeline_category::RASTERIZATION, std::move(target))
        {}


        void draw(const draw_data& data) override {
            VE_PROFILE_FN();

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
            postprocessing_pass->draw(quad_data);
        }
    private:
        void setup_pipeline(void) {
            VE_PROFILE_FN();


            auto get_shader = [] <typename V> (meta::type_wrapper<V> vertex, const auto& name, const auto&... stages) {
                return shader_cache::instance().template get_or_load_shader<V>(
                    std::vector<fs::path>{ io::paths::PATH_SHADERS / stages... },
                    name
                );
            };

            auto geometry_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::material_vertex_3d>{},
                "pbr_multi_pass_geometry",
                "pbr_multi_pass.vert.glsl", "pbr_multi_pass_geometry.frag.glsl"
            );

            auto lighting_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::no_vertex>{},
                "pbr_multi_pass_lighting",
                "pbr_multi_pass.g_input.glsl", "pbr_multi_pass_lighting.frag.glsl"
            );

            auto postprocessing_pass_shader = get_shader(
                meta::type_wrapper<vertex_types::no_vertex>{},
                "pbr_multi_pass_postprocessing",
                "pbr_multi_pass.g_input.glsl", "pbr_multi_pass_postprocessing.frag.glsl"
            );


            auto make_chain_subpass = [] (const auto& shader, const auto& prefix, bool depth, const auto&... color) {
                auto result = pipeline_chain_subpass { shader, prefix };

                if (depth) result.add_depth_attachment("depth");
                result.add_color_attachments({ color... });

                return result;
            };

            auto subpasses = create_pipeline_chain(
                {
                    make_chain_subpass(geometry_pass_shader,       "",   true,  "position", "normal", "color", "material"),
                    make_chain_subpass(lighting_pass_shader,       "g_", false, "position", "color"),
                    make_chain_subpass(postprocessing_pass_shader, "l_", false)
                },
                get_target()
            );

            geometry_pass       = std::move(subpasses[0]);
            lighting_pass       = std::move(subpasses[1]);
            postprocessing_pass = std::move(subpasses[2]);
        }


        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> postprocessing_pass;
    };
}