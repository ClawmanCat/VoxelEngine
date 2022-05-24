#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/any_of.hpp>
#include <VoxelEngine/utility/traits/evaluate_if_valid.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/target/target_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/renderpass_transforms.hpp>


namespace ve::gfx::opengl {
    struct renderpass_definition {
        using name_transform_t = std::function<std::string(std::string_view)>;
        using size_transform_t = std::function<vec2ui(vec2ui)>;

        shared<class shader> shader;
        std::vector<std::pair<std::size_t, std::string>> relative_inputs;
        std::vector<shared<uniform_sampler>> direct_inputs;
        std::vector<framebuffer_attachment_template> outputs;
        name_transform_t name_transform; // Mapping from previous stage attachment name to uniform name for this stage's input.
        size_transform_t size_transform = size_transforms::identity{}; // Size of this framebuffer as a function of the final target size.
        std::optional<std::string> pipeline_name = std::nullopt; // If no name is provided, the name of the shader is used.


        explicit renderpass_definition(shared<class shader> shader, name_transform_t transform = name_transforms::identity{}) :
            shader(std::move(shader)),
            name_transform(std::move(transform))
        {}


        // Gets the name the pipeline will have.
        // This is the provided name if there is one, or the name of the shader otherwise.
        std::string_view get_name(void) const {
            return pipeline_name ? *pipeline_name : shader->get_reflection().name;
        }


        // Modify the given attachment using the predicate. Must be callable as fn(framebuffer_attachment_template&).
        void modify_attachment(std::string_view name, auto fn) {
            auto it = ranges::find(outputs, name, &framebuffer_attachment_template::name);
            VE_DEBUG_ASSERT(it != outputs.end(), "No such attachment: ", name);

            std::invoke(fn, *it);
        }


        void add_attachment(framebuffer_attachment_template attachment) {
            outputs.emplace_back(std::move(attachment));
        }

        void add_color_attachment(std::string name, const texture_format& fmt = texture_format_RGBA32F) {
            add_attachment(framebuffer_attachment_template { std::move(name), framebuffer_attachment_template::COLOR_BUFFER, &fmt });
        }

        void add_color_attachments(const std::vector<std::string>& names, const texture_format& fmt = texture_format_RGBA32F) {
            for (const auto& name : names) {
                add_attachment(framebuffer_attachment_template { name, framebuffer_attachment_template::COLOR_BUFFER, &fmt });
            }
        }

        void add_depth_attachment(std::string name, const texture_format& fmt = texture_format_DEPTH32F) {
            add_attachment(framebuffer_attachment_template { std::move(name), framebuffer_attachment_template::DEPTH_BUFFER, &fmt });
        }


        // Adds an input from one of the outputs of a different renderpass.
        void add_indirect_input(std::size_t from_renderpass, std::string name) {
            relative_inputs.emplace_back(from_renderpass, std::move(name));
        }

        // Adds an input from some external uniform sampler.
        void add_indirect_input(shared<uniform_sampler> sampler) {
            direct_inputs.emplace_back(std::move(sampler));
        }

        // Adds an input from some external framebuffer attachment.
        // Optionally, a name transform can also be provided here instead of separately.
        void add_indirect_input(framebuffer* fbo, std::string attachment, std::optional<std::string> name_transform = std::nullopt) {
            if (!name_transform) name_transform = attachment;

            auto texture = fbo->get_attachments().at(attachment).texture;
            direct_inputs.emplace_back(make_shared<named_texture>(std::move(texture), std::move(*name_transform)));
        }
    };


    struct target_validators {
        std::function<vec2ui(void)> texture_validator;
        std::function<bool(void)> render_validator;
    };

    template <typename T> concept target_or_validators = meta::is_any_of_v<T, shared<render_target>, target_validators>;


    // Creates a set of pipelines with associated targets based on the provided renderpass data and chain them together,
    // using the outputs of each subpass as inputs for the next.
    // A final target may be provided, in which case the final renderpass will output to this target, and can't have any attachments itself.
    // If such a target is provided, the texture- and render-validators from that target will be used for all passes.
    // If no target is provided, texture- and render-validators must be provided manually.
    inline std::vector<shared<single_pass_pipeline>> build_pipeline(std::vector<renderpass_definition> passes, target_or_validators auto target) {
        std::vector<shared<single_pass_pipeline>> result;

        constexpr bool has_final_target = std::is_same_v<decltype(target), shared<render_target>>;
        auto texture_validator = ve_eval_if_else_c(has_final_target, target->get_texture_validator(), target.texture_validator);
        auto render_validator  = ve_eval_if_else_c(has_final_target, target->get_render_validator(),  target.render_validator);

        for (auto [i, pass] : passes | views::enumerate) {
            const bool is_last = has_final_target && (i == passes.size() - 1);
            const bool empty   = pass.outputs.empty();

            VE_DEBUG_ASSERT(
                !is_last || empty,
                "Output pass should use the outputs of the provided render target and not specify any attachments itself."
            );


            auto pass_tex_validator = [v_final = texture_validator, v_pass = pass.size_transform] { return v_pass(v_final()); };

            auto pass_target = is_last
                ? ve_eval_if_else_c(has_final_target, std::move(target), nullptr)
                : make_shared<render_target>(pass.outputs, std::move(pass_tex_validator), render_validator);

            result.emplace_back(single_pass_pipeline::create(
                std::move(pass_target),
                pass.shader,
                pass.pipeline_name ? *pass.pipeline_name : pass.shader->get_reflection().name
            ));


            // Previous pass' outputs should be inputs for this pass.
            if (i > 0) {
                const auto& prev = result[i - 1];

                for (const auto& [name, texture] : prev->get_target()->get_attachments()) {
                    result.back()->set_uniform_producer(make_shared<active_target_attachment>(prev, name, pass.name_transform(name)));
                }
            }

            // Pass may take arbitrary inputs from previous passes.
            for (const auto& [take_from_pass, name] : pass.relative_inputs) {
                VE_MAKE_CAPTURABLE(i);
                VE_MAKE_CAPTURABLE(take_from_pass);
                VE_ASSERT(VE_CAPTURE(take_from_pass) < VE_CAPTURE(i), "Render pass inputs can only be taken from passes executed before the current pass");

                result.back()->set_uniform_producer(make_shared<active_target_attachment>(result[take_from_pass], name, pass.name_transform(name)));
            }

            // Pass may take arbitrary textures as inputs.
            for (auto sampler : pass.direct_inputs) {
                auto new_name = pass.name_transform(sampler->get_uniform_name());

                auto renamed_sampler = (new_name == sampler->get_uniform_name())
                    ? std::move(sampler)
                    : make_shared<renaming_sampler>(std::move(new_name), std::move(sampler));

                result.back()->set_uniform_producer(std::move(renamed_sampler));
            }
        }

        return result;
    }
}