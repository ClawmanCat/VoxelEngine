#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/lighting/gaussian_blur.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>
#include <VoxelEngine/graphics/uniform/uniform_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/mixins/pipeline_mixin.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/renderpass_transforms.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline_builder.hpp>
#include <VoxelEngine/platform/graphics/opengl/utility/quad.hpp>


#define ve_impl_invalidating_setter(name, ret_type, keep_existing)  \
[[nodiscard]] ret_type get_##name(void) const { return name; }      \
                                                                    \
void set_##name(ret_type value) {                                   \
    name = value;                                                   \
    create_passes(keep_existing);                                   \
}


namespace ve::gfx::opengl {
    // TODO: Some of the setters require rebuilding the entire pipeline, this is currently not handled.
    template <typename Pipeline>
    class pipeline_bloom_mixin : public pipeline_mixin<pipeline_bloom_mixin<Pipeline>, Pipeline> {
    public:
        void on_construct(void) {
            static_assert(
                std::is_base_of_v<multipass_pipeline, Pipeline>,
                "The Bloom Pipeline Mixin can only be used multipass pipelines."
            );


            // Modify pipeline definition to include bloom passes before it is built.
            on_pre_build_stages = get_pipeline().add_handler([this] (const pbr_pipeline_pre_build_stages_event& e) {
                inject_passes(*e.renderpass_defs);
            });

            // Run bloom passes after the stage we take our inputs from.
            on_post_build_stages = get_pipeline().add_handler([this] (const pbr_pipeline_post_build_stages_event& e) {
                inject_draw_callback(*e.renderpasses);
            });

            // Construct bloom passes after the rest of the pipeline is created.
            on_post_build_pipeline = get_pipeline().add_handler([this] (const pbr_pipeline_post_build_event& e) {
                create_passes(false);
            });

            // Assert the pipeline is set up correctly to use bloom before drawing.
            on_pre_draw = get_pipeline().add_handler([this] (const pipeline_pre_draw_event& e) {
                check_pipeline_state(*e.draw_data);
            });
        }


        // Bloom settings. If using the bloom ECS mixin, these are set automatically.
        ve_impl_invalidating_setter(num_bloom_passes, u32,   true);
        ve_impl_invalidating_setter(scale_factor,     float, false);

        // Mutators for bloom shader settings. These should correspond to the used bloom shader.
        ve_impl_invalidating_setter(bloom_luma_attachment, const std::string&, false);
        ve_impl_invalidating_setter(bloom_luma_sampler,    const std::string&, false);
        ve_impl_invalidating_setter(pixavg_texture_array,  const std::string&, false);
        ve_impl_invalidating_setter(bloom_direction_ubo,   const std::string&, false);

        // Mutators for where to sample inputs from for the bloom shader.
        ve_impl_invalidating_setter(sample_luma_from, const std::string&, false);
        ve_impl_invalidating_setter(luma_attachment,  const std::string&, false);

        // Mutators for where the bloom shader sends its results to.
        ve_impl_invalidating_setter(write_bloom_to,          const std::string&, false);
        ve_impl_invalidating_setter(bloom_output_attachment, const std::string&, false);

        // Other accessors.
        VE_GET_CREF(bloom_passes);
        VE_GET_CREF(merge_pass);
    private:
        // Sampler object that takes all bloom pass outputs and returns them as a GLSL array.
        struct bloom_texture_array : public uniform_sampler {
            pipeline_bloom_mixin<Pipeline>* self;


            explicit bloom_texture_array(pipeline_bloom_mixin<Pipeline>* self) : uniform_sampler(), self(self) {}


            texture_list get_uniform_textures(void) const override {
                texture_list result;
                result.reserve(2 * self->num_bloom_passes);

                for (auto& [h, v] : self->bloom_passes) {
                    result.push_back(v->get_target()->get_attachment(self->bloom_luma_attachment).texture);
                }

                return result;
            }


            std::string get_uniform_name(void) const override {
                return self->pixavg_texture_array;
            }
        };

        friend struct bloom_texture_array;



        // Tokens for event handlers.
        event_handler_token on_pre_build_stages, on_post_build_stages, on_post_build_pipeline, on_pre_draw, on_prev_pre_draw;

        // Bloom shader attachment & sampler names.
        std::string bloom_luma_attachment  = "bloom";
        std::string bloom_luma_sampler     = "tex";
        std::string pixavg_texture_array   = "textures";
        std::string bloom_direction_ubo    = "U_GaussianDirection";

        // Names of bloom inputs and their pipelines.
        std::string sample_luma_from = "ve.pbr_lighting", luma_attachment = "bloom";
        // Name of the pipeline that will receive bloom data and its input.
        std::string write_bloom_to = "ve.pbr_postprocessing", bloom_output_attachment = "l_bloom";

        u32 num_bloom_passes = 4;
        float scale_factor   = 0.5f;

        struct bloom_pass { shared<single_pass_pipeline> horizontal, vertical; };
        std::vector<bloom_pass> bloom_passes;
        shared<single_pass_pipeline> merge_pass;




        // Retrieve a pipeline definition from the provided definitions by name.
        // If the pipeline is not found, "task" is used to display an error message (Debug mode only).
        renderpass_definition& get_external_pipeline(std::vector<renderpass_definition>& defs, std::string_view name, std::string_view task) {
            auto it = ranges::find_if(defs, [&] (const auto& def) { return def.get_name() == name; });
            VE_DEBUG_ASSERT(it != defs.end(), "Cannot find ", task, " renderpass for bloom mixin: ", name);

            return *it;
        }

        // Equivalent to above, but for pipelines that have already been built.
        shared<pipeline> get_external_pipeline(std::vector<shared<pipeline>>& pipelines, std::string_view name, std::string_view task) {
            auto it = ranges::find_if(pipelines, [&] (const auto& p) { return p->get_name() == name; });
            VE_DEBUG_ASSERT(it != pipelines.end(), "Cannot find ", task, " renderpass for bloom mixin: ", name);

            return *it;
        }




        void create_passes(bool keep_existing) {
            if (!keep_existing) bloom_passes.clear();


            auto gaussian_blur_shader = get_shader<vertex_types::no_vertex>(
                "ve.gaussian",
                "pipeline_common/screen_quad.g_input.glsl", "pipeline_common/gaussian.frag.glsl"
            );

            auto pixel_avg_shader = get_shader<vertex_types::no_vertex>(
                "ve.pixel_average",
                "pipeline_common/screen_quad.g_input.glsl", "pipeline_common/combine_textures.frag.glsl"
            );


            auto luma_source     = get_external_pipeline(get_pipeline().get_stages(), sample_luma_from,     "luma input");
            auto bloom_target    = get_external_pipeline(get_pipeline().get_stages(), write_bloom_to,       "luma output");


            std::vector<renderpass_definition> definitions;
            definitions.reserve(2 * num_bloom_passes + 1);

            // Add the required amount of bloom passes. Each bloom pass consists of a horizontal and a vertical pass.
            for (std::size_t i = bloom_passes.size(); i < num_bloom_passes; ++i) {
                for (bool is_horizontal : std::array { true, false }) {
                    const bool  is_first = (i == 0 && is_horizontal);
                    const float scale    = std::pow(scale_factor, float(i));


                    auto& def = definitions.emplace_back(renderpass_definition {
                        gaussian_blur_shader,
                        // For the first pass, use the name from the previous pipeline, otherwise use the bloom pass output.
                        name_transforms::rename_one {
                            .old_name = is_first ? luma_attachment : bloom_luma_attachment,
                            .new_name = bloom_luma_sampler
                        }
                    });


                    if (is_first) {
                        def.add_indirect_input(
                            luma_source->get_target().get(),
                            luma_attachment
                        );
                    }


                    def.add_color_attachment(bloom_luma_attachment);
                    def.modify_attachment(bloom_luma_attachment, [] (auto& tmpl) { tmpl.tex_filter = texture_filter::LINEAR; });
                    def.pipeline_name  = cat("bloom_", i, "_", is_horizontal ? "H" : "V");
                    def.size_transform = [scale] (vec2ui full_size) { return vec2ui { vec2f { full_size } * scale }; };
                }
            }

            // Merge (pixel average) the result of all bloom passes.
            auto& def = definitions.emplace_back(renderpass_definition { pixel_avg_shader });
            def.add_color_attachment(bloom_luma_attachment);
            def.modify_attachment(bloom_luma_attachment, [] (auto& tmpl) { tmpl.tex_filter = texture_filter::LINEAR; });
            def.add_indirect_input(make_shared<bloom_texture_array>(this));


            auto renderpasses = build_pipeline(
                definitions,
                target_validators {
                    .texture_validator = bloom_target->get_target()->get_texture_validator(), // Output same size as the stage that uses our bloom texture.
                    .render_validator  = bloom_target->get_target()->get_render_validator()   // Re-render whenever the stage that uses our bloom texture will be rendered.
                }
            );


            for (std::size_t i = 0; i < num_bloom_passes; ++i) {
                auto& pass = bloom_passes.emplace_back(bloom_pass {
                    .horizontal = std::move(renderpasses[2 * i + 0]),
                    .vertical   = std::move(renderpasses[2 * i + 1])
                });

                pass.horizontal->template set_uniform_value<u32>(bloom_direction_ubo, GAUSSIAN_HORIZONTAL);
                pass.vertical->template set_uniform_value<u32>(bloom_direction_ubo, GAUSSIAN_VERTICAL);
            }

            merge_pass = std::move(renderpasses.back());
            merge_pass->template set_uniform_value<u32>("num_populated_textures", num_bloom_passes);
        }




        void inject_passes(std::vector<renderpass_definition>& pipeline_defs) {
            auto& luma_source  = get_external_pipeline(pipeline_defs, sample_luma_from, "luma input");
            auto& bloom_target = get_external_pipeline(pipeline_defs, write_bloom_to,   "luma output");


            // Make sure source pass outputs luma.
            luma_source.add_color_attachment(luma_attachment);
            luma_source.modify_attachment(luma_attachment, [] (auto& tmpl) { tmpl.tex_filter = texture_filter::LINEAR; });


            // Make sure destination pass accepts luma as input.
            // Bloom passes may not yet exist at this point, so use a lazily initialized sampler input.
            bloom_target.add_indirect_input(make_shared<delayed_sampler>(
                bloom_luma_attachment,
                [this] {
                    return make_shared<named_texture>(
                        merge_pass->get_target()->get_attachment(bloom_luma_attachment).texture,
                        bloom_luma_attachment
                    );
                }
            ));

            bloom_target.name_transform = name_transforms::first_effective {
                name_transforms::rename_one { .old_name = bloom_luma_attachment, .new_name = bloom_output_attachment },
                std::move(bloom_target.name_transform)
            };


            // Make sure the entire pipeline is compiled with F_ENABLE_BLOOM = 1
            for (auto& def : pipeline_defs) {
                def.shader = def.shader->modify_and_recompile(
                    [] (auto& settings) { settings.preprocessor_definitions.insert_or_assign("F_ENABLE_BLOOM", "1"); }
                );
            }
        }




        void check_pipeline_state(const pipeline_draw_data& draw_data) {
            // Assert bloom data is provided.
            const static std::vector<std::string_view> required_mixins { "ve.bloom_mixin" };
            pipeline_assert_has_ecs_mixins(*draw_data.ctx, required_mixins);

            // Update settings from mixin.
            if (auto value = draw_data.ctx->template try_get_object<u32>("ve.bloom_mixin.num_bloom_passes"); value) {
                if (*value != num_bloom_passes) set_num_bloom_passes(*value);
            }

            if (auto value = draw_data.ctx->template try_get_object<float>("ve.bloom_mixin.scale_factor"); value) {
                if (*value != scale_factor) set_scale_factor(*value);
            }
        }




        void inject_draw_callback(std::vector<shared<pipeline>>& renderpasses) {
            auto luma_source = get_external_pipeline(renderpasses, sample_luma_from, "luma input");

            on_prev_pre_draw = luma_source->add_handler([this] (const pipeline_post_draw_event& e) {
                auto quad = draw_data_to_g_input(*e.draw_data);

                for (auto& [h, v] : bloom_passes) {
                    h->draw(quad);
                    v->draw(quad);
                }

                merge_pass->draw(quad);
            });
        }
    };
}