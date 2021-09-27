#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/vector.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>
#include <VoxelEngine/platform/graphics/vulkan/vertex/tensor_format.hpp>

#include <vulkan/vulkan.hpp>
#include <ctti/nameof.hpp>


namespace ve::gfx::vulkan {
    struct uniform_layout {
        vk_resource<VkDescriptorSetLayout> descriptor_set_layout;
        vk_resource<VkPipelineLayout> pipeline_layout;
    };


    struct vertex_layout {
        std::vector<VkVertexInputAttributeDescription> vertex_attributes;
        std::vector<VkVertexInputBindingDescription> vertex_bindings;
    };


    struct shader_layout {
        uniform_layout uniforms;
        vertex_layout vertex_attributes;
    };




    inline uniform_layout get_uniform_layout(const reflect::shader_reflection& reflection) {
        // Each UBO may appear in multiple stages, so make sure we don't get any duplicates by merging them.
        hash_map<std::string_view, VkDescriptorSetLayoutBinding> ubo_layouts;

        for (const auto& [stage, stage_reflection] : reflection.stages) {
            // TODO: Also support push constants.
            const bool no_push_constants = stage_reflection.push_constants.empty(); // Can't capture structured binding.
            VE_DEBUG_ASSERT(no_push_constants, "Push constants are not yet supported.");

            for (const auto& ubo : stage_reflection.uniform_buffers) {
                if (auto it = ubo_layouts.find(ubo.name); it != ubo_layouts.end()) {
                    it->second.stageFlags |= stage->vulkan_type;
                } else {
                    VkDescriptorSetLayoutBinding layout_binding {
                        .binding         = (u32) ubo.binding,
                        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags      = (VkShaderStageFlags) stage->vulkan_type
                    };

                    ubo_layouts.emplace(ubo.name, layout_binding);
                }
            }
        }

        std::vector<VkDescriptorSetLayoutBinding> ubo_layout_array = ubo_layouts
            | views::values
            | ranges::to<std::vector>;


        VkDescriptorSetLayoutCreateInfo ubo_layout_info {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = (u32) ubo_layout_array.size(),
            .pBindings    = ubo_layout_array.data()
        };

        VkDescriptorSetLayout ubo_layout;
        if (vkCreateDescriptorSetLayout(get_context()->logical_device, &ubo_layout_info, nullptr, &ubo_layout) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan descriptor set layout.");
        }


        VkPipelineLayoutCreateInfo pipeline_layout_info {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = 1,
            .pSetLayouts            = &ubo_layout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr
        };

        VkPipelineLayout pipeline_layout;
        if (vkCreatePipelineLayout(get_context()->logical_device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan pipeline layout.");
        }


        return {
            vk_resource<VkDescriptorSetLayout> {
                std::move(ubo_layout),
                bind<0, 2>(vkDestroyDescriptorSetLayout, *(get_context()->logical_device), nullptr)
            },

            vk_resource<VkPipelineLayout> {
                std::move(pipeline_layout),
                bind<0, 2>(vkDestroyPipelineLayout, *(get_context()->logical_device), nullptr)
            }
        };
    }




    template <typename Vertex>
    inline vertex_layout get_vertex_layout(const reflect::shader_reflection& reflection) {
        constexpr auto vertex_attributes = Vertex::get_vertex_layout();

        const auto& [input_stage, attributes] = reflection.get_input_stage();


        auto matches = combine_into_pairs(
            vertex_attributes,
            attributes.inputs,
            [](const auto& va, const auto& ia) { return va.name == ia.name; }
        );


        for (const auto& unmatched_va : matches.unmatched_a | views::indirect) {
            VE_LOG_WARN(cat(
                "Shader ", reflection.name, " has no input variable for vertex attribute ",
                unmatched_va.name, " in vertex of type ", ctti::nameof<Vertex>().cppstring(), "."
            ));
        }

        for (const auto& unmatched_ia : matches.unmatched_b | views::indirect) {
            VE_LOG_ERROR(cat(
                "Shader ", reflection.name, " requires input variable ", unmatched_ia.name,
                " but it is not provided by vertex of type ", ctti::nameof<Vertex>().cppstring(), "."
            ));
        }

        if(!matches.unmatched_b.empty()) {
            throw std::runtime_error { cat("Shader ", reflection.name, " has unfulfilled vertex inputs.") };
        }


        vertex_layout result;
        for (const auto& [vertex_attrib, input_attrib] : matches.matched) {
            result.vertex_bindings.push_back(VkVertexInputBindingDescription {
                .binding   = (u32) input_attrib->binding,
                .stride    = (u32) vertex_attrib->size,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            });

            result.vertex_attributes.push_back(VkVertexInputAttributeDescription {
                .location = (u32) input_attrib->location,
                .binding  = (u32) input_attrib->binding,
                // TODO: FIX THIS!
                .format   = VK_FORMAT_R32G32B32A32_SFLOAT, //detail::vulkan_format_for_attribute(*vertex_attrib),
                .offset   = (u32) vertex_attrib->offset
            });
        }


        return result;
    }
}