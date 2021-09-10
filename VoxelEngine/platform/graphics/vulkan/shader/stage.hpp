#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_category.hpp>

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>


namespace ve::gfx::vulkan {
    struct shader_stage {
        std::string name;
        std::string file_extension;

        shaderc_shader_kind shaderc_type;
        VkShaderStageFlagBits vulkan_type;

        const pipeline_category* pipeline;
        enum pipeline_mode_t { REQUIRED, OPTIONAL } pipeline_mode;

        bool first = false;
        bool last  = false;
    };


    // Yes, this could be inline. It could be an std::array with deduced size even,
    // were it not for the fact that Clang completely fails to parse the initializer list for some obscure reason
    // if it is declared here.
    const extern std::vector<shader_stage> shader_stages;
}