#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct pipeline_category {
        std::string name;
        queue_family::enum_t queue_family;
        VkPipelineBindPoint bindpoint;
    };


    namespace pipeline_categories {
        const static inline pipeline_category RASTERIZATION {
            .name         = "rasterization",
            .queue_family = queue_family::GRAPHICS_QUEUE
        };


        const static inline pipeline_category COMPUTE {
            .name         = "compute",
            .queue_family = queue_family::COMPUTE_QUEUE
        };
    }
}