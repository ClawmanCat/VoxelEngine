#include <VoxelEngine/platform/graphics/vulkan/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve::gfx::vulkan::detail {
    shader make_shader_impl(const shader_compilation_data& data, shader_layout&& layout) {
        VE_ASSERT(!data.spirv.empty(), "Shader must consist of at least one stage.");

        const auto* pipeline = data.spirv.begin()->first->pipeline;
        VE_ASSERT(
            ranges::all_of(data.spirv | views::keys | views::indirect, equal_on(&gfxapi::shader_stage::pipeline, pipeline)),
            "All shader stages must be part of the same pipeline."
        );


        std::vector<shader_module_info> modules;

        for (const auto& [stage, spirv] : data.spirv) {
            VkShaderModuleCreateInfo module_info {
                .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = spirv.size() * sizeof(typename std::remove_cvref_t<decltype(spirv)>::value_type),
                .pCode    = spirv.data()
            };

            VkShaderModule module;
            if (vkCreateShaderModule(get_context()->logical_device, &module_info, nullptr, &module) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to create Vulkan shader module.");
            }


            VkPipelineShaderStageCreateInfo pipeline_info {
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = stage->vulkan_type,
                .module = module,
                .pName  = "main"
            };


            modules.push_back(shader_module_info {
                vk_resource<VkShaderModule> {
                    std::move(module),
                    bind<0, 2>(vkDestroyShaderModule, *(get_context()->logical_device), nullptr)
                },
                pipeline_info
            });
        }


        return shader {
            data.reflection,
            std::move(modules),
            pipeline,
            std::move(layout)
        };
    }
}