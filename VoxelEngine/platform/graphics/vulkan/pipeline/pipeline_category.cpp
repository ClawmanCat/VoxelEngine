#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_category.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline.hpp>


namespace ve::gfx::vulkan::detail {
    vk_resource<VkPipeline> create_graphics_pipeline(const pipeline_create_data& data) {
        const vec2ui size = data.targets[0].fb->get_image().get_size();

        VkViewport viewport {
            .x = 0.0f, .y = 0.0f,
            .width    = (float) size.x,
            .height   = (float) size.y,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor {
            .offset = { 0, 0 },
            .extent = { size.x, size.y }
        };

        VkPipelineViewportStateCreateInfo viewport_info {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports    = &viewport,
            .scissorCount  = 1,
            .pScissors     = &scissor
        };


        VkPipelineInputAssemblyStateCreateInfo input_assembly_info {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = (VkPrimitiveTopology) data.settings.topology,
            .primitiveRestartEnable = VK_FALSE
        };


        VkPipelineRasterizationStateCreateInfo rasterization_info {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = data.settings.clamp_depth,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = (VkPolygonMode) data.settings.polygon_mode,
            .cullMode                = (VkCullModeFlags) data.settings.cull_mode,
            .frontFace               = (VkFrontFace) data.settings.cull_direction,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = data.settings.line_width
        };


        VkPipelineMultisampleStateCreateInfo multisampling_info {
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable  = VK_FALSE
        };


        VkPipelineColorBlendAttachmentState blending_info {
            .blendEnable    = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };


        std::array dynamic_state {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
            VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
            VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
            VK_DYNAMIC_STATE_FRONT_FACE_EXT,
            VK_DYNAMIC_STATE_CULL_MODE_EXT,
            VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_info {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = dynamic_state.size(),
            .pDynamicStates    = dynamic_state.data()
        };



    }


    vk_resource<VkPipeline> create_compute_pipeline(const pipeline_create_data&) {
        VE_NOT_YET_IMPLEMENTED;
    }
}