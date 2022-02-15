#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target_sampler.hpp>
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
            draw_data quad_data {
                .buffers         = { g_input_quad().get() },
                .lighting_target = data.lighting_target,
                .lights          = data.lights,
                .ambient_light   = data.ambient_light,
                .ctx             = data.ctx
            };


            geometry_pass->draw(data);
            lighting_pass->draw(quad_data);
            postprocessing_pass->draw(quad_data);
        }
    private:
        void setup_pipeline(void) {
            VE_PROFILE_FN();

            auto attachment_template = [](const auto& name, bool color = true) {
                return framebuffer_attachment {
                    name,
                    color ? framebuffer_attachment::COLOR_BUFFER : framebuffer_attachment::DEPTH_BUFFER,
                    color ? &texture_format_RGBA32F : &texture_format_DEPTH32F
                };
            };


            auto g_buffer = make_shared<render_target>(
                std::vector {
                    attachment_template("position"),
                    attachment_template("normal"),
                    attachment_template("color"),
                    attachment_template("material"),
                    attachment_template("depth", false)
                },
                get_target()->get_texture_validator(),
                get_target()->get_render_validator()
            );

            geometry_pass = make_shared<single_pass_pipeline>(
                std::move(g_buffer),
                shader_cache::instance().get_or_load_shader<vertex_types::material_vertex_3d>(
                    std::vector<fs::path> {
                        io::paths::PATH_SHADERS / "pbr_multi_pass.vert.glsl",
                        io::paths::PATH_SHADERS / "pbr_multi_pass_geometry.frag.glsl",
                    },
                    "pbr_multi_pass_geometry"
                )
            );


            auto l_buffer = make_shared<render_target>(
                std::vector {
                    attachment_template("position"),
                    attachment_template("color"),
                },
                get_target()->get_texture_validator(),
                get_target()->get_render_validator()
            );

            lighting_pass = make_shared<single_pass_pipeline>(
                std::move(l_buffer),
                shader_cache::instance().get_or_load_shader<vertex_types::no_vertex>(
                    std::vector<fs::path> {
                        io::paths::PATH_SHADERS / "pbr_multi_pass.g_input.glsl",
                        io::paths::PATH_SHADERS / "pbr_multi_pass_lighting.frag.glsl",
                    },
                    "pbr_multi_pass_lighting"
                )
            );

            // Use the outputs of the geometry pass as inputs for the lighting pass.
            for (const auto& name : make_array<std::string>("position", "color", "normal", "material")) {
                lighting_pass->set_uniform_producer(make_shared<active_target_attachment>(geometry_pass, name, "g_"s + name));
            }


            postprocessing_pass = make_shared<single_pass_pipeline>(
                get_target(),
                shader_cache::instance().get_or_load_shader<vertex_types::no_vertex>(
                    std::vector<fs::path> {
                        io::paths::PATH_SHADERS / "pbr_multi_pass.g_input.glsl",
                        io::paths::PATH_SHADERS / "pbr_multi_pass_postprocessing.frag.glsl",
                    },
                    "pbr_multi_pass_postprocessing"
                )
            );

            // Bind the outputs of the lighting pass as samplers of the postprocessing pass.
            for (const auto& name : make_array<std::string>("position", "color")) {
                postprocessing_pass->set_uniform_producer(make_shared<active_target_attachment>(lighting_pass, name, "l_"s + name));
            }
        }


        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> postprocessing_pass;
    };
}