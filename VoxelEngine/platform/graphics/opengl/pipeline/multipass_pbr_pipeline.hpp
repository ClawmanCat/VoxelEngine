#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/graphics/shader/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
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
            raii_tasks on_destruct;


            data.ctx->pipelines.push(this);
            on_destruct.add_task([&] { data.ctx->pipelines.pop(); });
            data.ctx->uniform_state.push_uniforms(this);
            on_destruct.add_task([&] { data.ctx->uniform_state.pop_uniforms(); });


            // Since we're doing one render pass per light, we don't have one UBO for all lighting data.
            // Ambient light must be handled separately.
            uniform_storage::set_uniform_value<vec3f>("ambient_light", data.ambient_light);


            // Note: the target of the postprocessing pass is the final target, so don't clear that, in case other stuff is rendered to it as well.
            geometry_pass->get_target()->clear();
            lighting_pass->get_target()->clear();


            geometry_pass->draw(data);


            // Same as draw data, but with the screen-filling quad as its only vertex buffer.
            draw_data quad_data {
                .buffers         = { g_input_quad().get() },
                .lighting_target = data.lighting_target,
                .lights          = data.lights,
                .ambient_light   = data.ambient_light,
                .ctx             = data.ctx
            };


            for (const auto& [i, light] : data.lights | views::enumerate) {
                uniform_storage::set_uniform_value<light_source>("light", light);

                lighting_pass->draw(quad_data);

                has_rendered_l_buffer = true;
                on_destruct.add_task([&] { has_rendered_l_buffer = false; });

                swap_l_buffers();
            }


            postprocessing_pass->draw(quad_data);
        }
    private:
        void setup_pipeline(void) {
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


            for (auto& l_buffer : l_buffers) {
                l_buffer = make_shared<render_target>(
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
            }

            lighting_pass = make_shared<single_pass_pipeline>(
                l_buffers[active_l_buffer],
                shader_cache::instance().get_or_load_shader<vertex_types::no_vertex>(
                    std::vector<fs::path> {
                        io::paths::PATH_SHADERS / "pbr_multi_pass.g_input.glsl",
                        io::paths::PATH_SHADERS / "pbr_multi_pass_lighting.frag.glsl",
                    },
                    "pbr_multi_pass_lighting"
                )
            );

            // - For the first lighting pass, use the outputs of the geometry pass.
            // - For subsequent passes, alternate using the different L buffers.
            for (const auto& name : make_array<std::string>("position", "color", "normal", "material")) {
                lighting_pass->set_uniform_producer(make_shared<lighting_data_source>(shared_from_this(), name, "g_"s + name));
            }

            // Also provide the output of the geometry pass directly.
            lighting_pass->set_uniform_producer(make_shared<active_target_attachment>(geometry_pass, "color", "g_color_initial"));


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
                postprocessing_pass->set_uniform_producer(make_shared<lighting_data_source>(shared_from_this(), name, "l_"s + name));
            }
        }


        void swap_l_buffers(void) {
            active_l_buffer ^= 1;
            lighting_pass->set_target(l_buffers[active_l_buffer]);
        }


        // The lighting render pass requires a different data source on each invocation:
        // For the first invocation, we take the data from the G buffer. After that we alternate the L buffers.
        shared<render_target> get_lighting_data_source(void) {
            if (!has_rendered_l_buffer) {
                return geometry_pass->get_target();
            } else {
                // Note: this is the l_buffer we read from, so the *inactive* one.
                return l_buffers[active_l_buffer ^ 1];
            }
        }


        // 6 empty vertices used to render a quad in g_input shaders.
        static shared<vertex_buffer> g_input_quad(void) {
            static auto i = [] {
                auto result = unindexed_vertex_buffer<vertex_types::no_vertex>::create();
                result->store_vertices(std::vector(6, vertex_types::no_vertex { }));

                return result;
            } ();

            return i;
        };

        shared<single_pass_pipeline> geometry_pass;
        shared<single_pass_pipeline> lighting_pass;
        shared<single_pass_pipeline> postprocessing_pass;


        // Since the output from each light is used when rendering the next light, we need to use swap buffers.
        std::array<shared<render_target>, 2> l_buffers;
        u8 active_l_buffer = 0;
        bool has_rendered_l_buffer = false;


        struct lighting_data_source : public uniform_sampler {
            shared<multipass_pbr_pipeline> self;
            std::string input;
            std::string output;

            lighting_data_source(shared<multipass_pbr_pipeline> self, std::string input, std::string output) :
                self(std::move(self)), input(std::move(input)), output(std::move(output))
            {}

            texture_list get_uniform_textures(void) const override {
                return { self->get_lighting_data_source()->get_attachments().at(input) };
            }

            std::string get_uniform_name(void) const override {
                return output;
            }
        };

        friend struct get_active_l_buffer;


        struct active_target_attachment : public uniform_sampler {
            shared<single_pass_pipeline> source;
            std::string input;
            std::string output;

            active_target_attachment(shared<single_pass_pipeline> source, std::string input, std::string output) :
                source(std::move(source)), input(std::move(input)), output(std::move(output))
            {}

            texture_list get_uniform_textures(void) const override {
                return { source->get_target()->get_attachments().at(input) };
            }

            std::string get_uniform_name(void) const override {
                return output;
            }
        };
    };
}