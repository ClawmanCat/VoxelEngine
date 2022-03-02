#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/target_sampler.hpp>


namespace ve::gfx::opengl {
    struct pipeline_chain_subpass {
        using transform_fn_t = std::function<std::string(std::string_view)>;

        shared<class shader> shader;
        std::vector<framebuffer_attachment> outputs;
        transform_fn_t name_transform; // Mapping from previous stage attachment name to uniform name for this stage's input.


        explicit pipeline_chain_subpass(shared<class shader> shader, transform_fn_t transform = cast<std::string>(), std::vector<framebuffer_attachment> outputs = {}) :
            shader(std::move(shader)),
            outputs(std::move(outputs)),
            name_transform(std::move(transform))
        {}


        // Overload for common case where the transform is simply adding a prefix, e.g. color => g_color.
        explicit pipeline_chain_subpass(shared<class shader> shader, std::string input_prefix, std::vector<framebuffer_attachment> outputs = {}) :
            shader(std::move(shader)),
            outputs(std::move(outputs)),
            name_transform([p = std::move(input_prefix)] (std::string_view sv) { return cat(p, sv); })
        {}


        void add_attachment(framebuffer_attachment attachment) {
            outputs.emplace_back(std::move(attachment));
        }

        void add_color_attachment(std::string name, const texture_format& fmt = texture_format_RGBA32F) {
            add_attachment(framebuffer_attachment { std::move(name), framebuffer_attachment::COLOR_BUFFER, &fmt });
        }

        void add_color_attachments(const std::vector<std::string>& names, const texture_format& fmt = texture_format_RGBA32F) {
            for (const auto& name : names) {
                add_attachment(framebuffer_attachment { name, framebuffer_attachment::COLOR_BUFFER, &fmt });
            }
        }

        void add_depth_attachment(std::string name, const texture_format& fmt = texture_format_DEPTH32F) {
            add_attachment(framebuffer_attachment { std::move(name), framebuffer_attachment::DEPTH_BUFFER, &fmt });
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

            VE_DEBUG_ASSERT(
                !is_last || pass.outputs.empty(),
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
        }

        return result;
    }
}