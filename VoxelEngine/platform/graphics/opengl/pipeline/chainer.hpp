#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target_sampler.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/chainer_name_transforms.hpp>


namespace ve::gfx::opengl {
    struct pipeline_chain_subpass {
        using transform_fn_t = std::function<std::string(std::string_view)>;

        shared<class shader> shader;
        std::vector<std::pair<std::size_t, std::string>> relative_inputs;
        std::vector<named_texture> direct_inputs;
        std::vector<framebuffer_attachment> outputs;
        transform_fn_t name_transform; // Mapping from previous stage attachment name to uniform name for this stage's input.


        explicit pipeline_chain_subpass(shared<class shader> shader, transform_fn_t transform = name_transforms::identity{}, std::vector<framebuffer_attachment> outputs = {}) :
            shader(std::move(shader)),
            outputs(std::move(outputs)),
            name_transform(std::move(transform))
        {}


        pipeline_chain_subpass& add_attachment(framebuffer_attachment attachment) {
            outputs.emplace_back(std::move(attachment));
            return *this; // Allow chaining.
        }

        pipeline_chain_subpass& add_color_attachment(std::string name, const texture_format& fmt = texture_format_RGBA32F) {
            add_attachment(framebuffer_attachment { std::move(name), framebuffer_attachment::COLOR_BUFFER, &fmt });
            return *this; // Allow chaining.
        }

        pipeline_chain_subpass& add_color_attachments(const std::vector<std::string>& names, const texture_format& fmt = texture_format_RGBA32F) {
            for (const auto& name : names) {
                add_attachment(framebuffer_attachment { name, framebuffer_attachment::COLOR_BUFFER, &fmt });
            }

            return *this; // Allow chaining.
        }

        pipeline_chain_subpass& add_depth_attachment(std::string name, const texture_format& fmt = texture_format_DEPTH32F) {
            add_attachment(framebuffer_attachment { std::move(name), framebuffer_attachment::DEPTH_BUFFER, &fmt });
            return *this; // Allow chaining.
        }


        // Adds an input from one of the outputs of a different renderpass.
        pipeline_chain_subpass& add_indirect_input(std::size_t from_renderpass, std::string name) {
            relative_inputs.emplace_back(from_renderpass, std::move(name));
            return *this; // Allow chaining.
        }

        // Adds an input from some external texture.
        pipeline_chain_subpass& add_indirect_input(named_texture texture) {
            direct_inputs.emplace_back(std::move(texture));
            return *this; // Allow chaining.
        }

        // Adds an input from some external framebuffer attachment.
        pipeline_chain_subpass& add_indirect_input(framebuffer* fbo, std::string attachment) {
            auto texture = fbo->get_attachments().at(attachment);
            direct_inputs.emplace_back(named_texture { std::move(texture), std::move(attachment) });

            return *this; // Allow chaining.
        }
    };


    // Creates a set of pipelines with associated targets based on the provided subpass data and chain them together,
    // using the outputs of each subpass as inputs for the next.
    inline std::vector<shared<single_pass_pipeline>> create_pipeline_chain(std::vector<pipeline_chain_subpass> passes, shared<render_target> target) {
        std::vector<shared<single_pass_pipeline>> result;

        auto texture_validator = target->get_texture_validator();
        auto render_validator  = target->get_render_validator();

        for (auto [i, pass] : passes | views::enumerate) {
            const bool is_last = (i == passes.size() - 1);
            const bool empty   = pass.outputs.empty();

            VE_DEBUG_ASSERT(
                !is_last || empty,
                "Output pass should use the outputs of the provided render target and not specify any attachments itself."
            );


            auto pass_target = is_last
                ? std::move(target)
                : make_shared<render_target>(pass.outputs, texture_validator, render_validator);

            result.emplace_back(make_shared<single_pass_pipeline>(std::move(pass_target), std::move(pass.shader)));


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
            for (const auto& named_tex : pass.direct_inputs) {
                result.back()->set_uniform_producer(make_shared<named_texture>(named_tex));
            }
        }

        return result;
    }
}