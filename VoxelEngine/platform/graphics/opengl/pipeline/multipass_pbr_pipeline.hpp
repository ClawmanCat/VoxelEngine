#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/vertex_buffer.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/graphics/shader/cache.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace ve::gfx::opengl {
    // TODO: Migrate to generic class for multipass pipelines.
    class multipass_pbr_pipeline : public pipeline, public std::enable_shared_from_this<multipass_pbr_pipeline> {
    public:
        ve_shared_only_then(multipass_pbr_pipeline, setup_pipeline, shared<render_target> target) :
            pipeline(&pipeline_category::RASTERIZATION, std::move(target))
        {}


        void draw(const draw_data& data) override {
            data.ctx->pipelines.push(this);
            data.ctx->uniform_state.push_uniforms(this);

            // Note: the target of the postprocessing pass is the final target, so don't clear that, in case other stuff is rendered to it as well.
            geometry_pass->get_target()->clear();
            lighting_pass->get_target()->clear();


            geometry_pass->draw(data);


            // Assure we always run the lighting pass once, even if there are no lights, so there is data in the l buffer.
            auto lights = data.lights;

            if (lights.empty()) {
                lights.push_back(light_source { .position = vec3f { 0 }, .radiance = vec3f { 0 }, .attenuation = 1.0f });
            }

            for (const auto& [i, light] : lights | views::enumerate) {
                uniform_storage::set_uniform_value<light_source>("light", light);

                draw_data data_for_light {
                    .buffers       = { g_input_quad().get() },
                    .ambient_light = data.ambient_light,
                    .ctx           = data.ctx
                };

                lighting_pass->draw(data_for_light);

                // Don't swap after rendering the last light, we want to use the final texture as input for the next stage.
                if (i + 1 < lights.size()) swap_l_buffers();
            }


            draw_data data_for_postprocessing {
                .buffers         = { g_input_quad().get() },
                .lighting_target = data.lighting_target,
                .lights          = data.lights,
                .ambient_light   = data.ambient_light,
                .ctx             = data.ctx
            };

            postprocessing_pass->draw(data_for_postprocessing);


            data.ctx->uniform_state.pop_uniforms();
            data.ctx->pipelines.pop();
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

            // Bind the outputs of the geometry pass as samplers of the lighting pass.
            for (const auto& name : make_array<std::string>("position", "normal", "material")) {
                lighting_pass->set_uniform_producer(make_shared<get_active_target_attachment>(geometry_pass, name, "g_"s + name));
            }

            // Color is special, since we edit it when rendering each light, and then re-use it.
            lighting_pass->set_uniform_producer(make_shared<get_active_l_buffer>(shared_from_this(), "color", "g_color"));


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
                postprocessing_pass->set_uniform_producer(make_shared<get_active_target_attachment>(lighting_pass, name, "l_"s + name));
            }
        }


        void swap_l_buffers(void) {
            active_l_buffer ^= 1;
            lighting_pass->set_target(l_buffers[active_l_buffer]);
        }


        // 4 empty vertices used to render a quad in g_input shaders.
        static shared<vertex_buffer> g_input_quad(void) {
            static auto i = [] {
                auto result = unindexed_vertex_buffer<vertex_types::no_vertex>::create();
                result->store_vertices(std::vector(4, vertex_types::no_vertex { }));

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


        struct get_active_l_buffer : public uniform_sampler {
            shared<multipass_pbr_pipeline> self;
            std::string input;
            std::string output;

            get_active_l_buffer(shared<multipass_pbr_pipeline> self, std::string input, std::string output) :
                self(std::move(self)), input(std::move(input)), output(std::move(output))
            {}

            texture_list get_uniform_textures(void) const override {
                return { self->l_buffers[self->active_l_buffer]->get_attachments().at(input) };
            }

            std::string get_uniform_name(void) const override {
                return output;
            }
        };

        friend struct get_active_l_buffer;


        struct get_active_target_attachment : public uniform_sampler {
            shared<single_pass_pipeline> source;
            std::string input;
            std::string output;

            get_active_target_attachment(shared<single_pass_pipeline> source, std::string input, std::string output) :
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