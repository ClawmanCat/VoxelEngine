#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_settings.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/framebuffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/shader/shader.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    namespace detail {
        // A framebuffer plus a name, corresponding to an output of the final shader stage.
        // Outputs from the shader to the given name will be written to the given framebuffer.
        struct named_framebuffer {
            const framebuffer* fb;
            std::string name;
        };


        struct pipeline_create_data {
            const std::vector<named_framebuffer>& targets;
            const pipeline_settings& settings;
            shared<shader> shader_program;
        };
    }


    // TODO: Handle pipelines with multiple subpasses.
    // This would need some extra abstraction for compute pipelines, as they currently don't support multi-pass rendering.
    inline vk_resource<VkPipeline> create_pipeline(
        const std::vector<detail::named_framebuffer>& targets,
        const pipeline_settings& settings,
        shared<shader> shader_program
    ) {
        VE_ASSERT(!targets.empty(), "Cannot create pipeline without any targets.");

        VE_ASSERT(
            ranges::all_of(
                targets | views::transform(ve_get_field(fb)) | views::indirect | views::transform(ve_get_field(get_image())),
                equal_on(&image::get_size, targets[0].fb->get_image().get_size())
            ),
            "Cannot create pipeline from targets not sharing a common size."
        );

        return shader_program->get_pipeline()->create_fn({ targets, settings, shader_program });
    }
}