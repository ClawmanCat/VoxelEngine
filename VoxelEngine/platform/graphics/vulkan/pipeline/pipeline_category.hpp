#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    namespace detail {
        struct pipeline_create_data;

        extern vk_resource<VkPipeline> create_graphics_pipeline(const pipeline_create_data&);
        extern vk_resource<VkPipeline> create_compute_pipeline(const pipeline_create_data&);
    }


    struct pipeline_category {
        std::string name;
        queue_family::enum_t queue_family;
        VkPipelineBindPoint bindpoint;

        fn<vk_resource<VkPipeline>, const detail::pipeline_create_data&> create_fn;
    };


    namespace pipeline_categories {
        const static inline pipeline_category RASTERIZATION {
            .name         = "rasterization",
            .queue_family = queue_family::GRAPHICS_QUEUE,
            .create_fn    = detail::create_graphics_pipeline
        };


        const static inline pipeline_category COMPUTE {
            .name         = "compute",
            .queue_family = queue_family::COMPUTE_QUEUE,
            .create_fn    = detail::create_compute_pipeline
        };
    }
}