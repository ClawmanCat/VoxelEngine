#include <VoxelEngine/platform/graphics/vulkan/pipeline/rasterization_pipeline.hpp>
#include <VoxelEngine/platform/graphics/vulkan/pipeline/pipeline_category.hpp>


namespace ve::gfx::vulkan {
    void rasterization_pipeline::recreate_pipeline(
        const shared<shader>& shader_program,
        const shared<render_target>& target,
        const pipeline_settings& settings
    ) {
        // The set of images for each swapchain index are identical, so we can just use any index as a representative sample.
        auto attachments = target->get_attachments();
        VE_ASSERT(!attachments.empty(), "Cannot create pipeline to target without any attachments.");

        VE_ASSERT(target->get_swap_count() > 0, "Cannot create pipeline to target without any images.");
        auto attachment_samples = target->get_images_for_index(0);


        std::vector<image*> images;
        for (std::size_t i = 0; i < target->get_swap_count(); ++i) {
            auto images_for_index = target->get_images_for_index(i);
            images.insert(images.end(), images_for_index.begin(), images_for_index.end());
        }

        VE_ASSERT(
            ranges::all_of(images | views::indirect, equal_on(&image::get_size, images[0]->get_size())),
            "All images used in a pipeline must have the same dimensions."
        );



        const auto& layout = shader_program->get_layout();

        VkPipelineVertexInputStateCreateInfo vertex_input_info {
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = (u32) layout.vertex_attributes.vertex_bindings.size(),
            .pVertexBindingDescriptions      = layout.vertex_attributes.vertex_bindings.data(),
            .vertexAttributeDescriptionCount = (u32) layout.vertex_attributes.vertex_attributes.size(),
            .pVertexAttributeDescriptions    = layout.vertex_attributes.vertex_attributes.data()
        };


        // All images will have the same size so we can query any one of them.
        const vec2ui size = attachment_samples[0]->get_size();

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
            .topology               = (VkPrimitiveTopology) settings.topology,
            .primitiveRestartEnable = VK_FALSE
        };


        VkPipelineRasterizationStateCreateInfo rasterization_info {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = settings.clamp_depth,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = (VkPolygonMode) settings.polygon_mode,
            .cullMode                = (VkCullModeFlags) settings.cull_mode,
            .frontFace               = (VkFrontFace) settings.cull_direction,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = settings.line_width
        };


        VkPipelineMultisampleStateCreateInfo multisampling_info {
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable  = VK_FALSE
        };


        VkPipelineColorBlendAttachmentState blending_state {
            .blendEnable    = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo blending_info {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable   = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments    = &blending_state
        };


        VkPipelineDepthStencilStateCreateInfo depth_info {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable       = (VkBool32) settings.enable_depth,
            .depthWriteEnable      = (VkBool32) settings.enable_depth,
            .depthCompareOp        = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE
        };


        // Dynamic state for viewport is not included since our image objects store their size internally,
        // and it's easier to just recreate them with the swapchain if it resizes.
        std::array dynamic_state {
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


        const auto& [output_stage, output_attribs] = shader_program->get_reflection().get_output_stage();
        const auto& outputs = output_attribs.outputs; // Can't reference structured binding in assert lambda.

        for (std::size_t i = 0; i < outputs.size(); ++i) {
            VE_ASSERT(
                ranges::contains(outputs | views::transform(ve_get_field(binding)), i),
                "Named shader outputs must have continuous binding indices."
            );
        }


        u32 named_index = 0;
        u32 unnamed_index = (u32) outputs.size();

        std::vector<VkAttachmentDescription> attachment_descriptions;
        std::vector<VkAttachmentReference> color_attachments, depth_attachments, input_attachments;

        for (const auto& [attachment, image] : views::zip(attachments, attachment_samples)) {
            attachment_descriptions.push_back(VkAttachmentDescription {
                .format         = image->get_pixel_format(),
                .samples        = VK_SAMPLE_COUNT_1_BIT,
                .loadOp         = attachment.get_load_op(),
                .storeOp        = attachment.get_store_op(),
                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout  = attachment.get_start_layout(),
                .finalLayout    = attachment.get_target_layout()
            });


            VkAttachmentReference reference {
                .attachment = attachment.is_named() ? named_index++ : unnamed_index++,
                .layout     = attachment.get_intermediate_layout()
            };

            switch (attachment.get_usage()) {
                case pipeline_attachment::COLOR_BUFFER:
                case pipeline_attachment::PRESENTABLE:
                    color_attachments.push_back(reference);
                    break;
                case pipeline_attachment::DEPTH_BUFFER:
                    depth_attachments.push_back(reference);
                    break;
                case pipeline_attachment::INPUT:
                    input_attachments.push_back(reference);
                    break;
                default:
                    VE_ASSERT(false, "Unsupported attachment usage.");
            }
        }

        VE_ASSERT(depth_attachments.size() < 2, "At most one depth attachment can be added to a rasterization pipeline.");


        VkSubpassDescription subpass_info {
            .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount    = (u32) input_attachments.size(),
            .pInputAttachments       = input_attachments.data(),
            .colorAttachmentCount    = (u32) color_attachments.size(),
            .pColorAttachments       = color_attachments.data(),
            .pDepthStencilAttachment = depth_attachments.data()
        };

        VkRenderPassCreateInfo renderpass_info {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = (u32) attachment_descriptions.size(),
            .pAttachments    = attachment_descriptions.data(),
            .subpassCount    = 1,
            .pSubpasses      = &subpass_info
        };

        VkRenderPass raw_renderpass;
        if (vkCreateRenderPass(get_context()->logical_device, &renderpass_info, nullptr, &raw_renderpass) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan render pass.");
        }

        this->renderpass = vk_resource<VkRenderPass> {
            std::move(raw_renderpass),
            ve::bind<0, 2>(vkDestroyRenderPass, get_context()->logical_device.value, nullptr)
        };


        std::vector<VkPipelineShaderStageCreateInfo> stages = shader_program->get_modules()
            | views::transform(ve_get_field(pipeline_info))
            | ranges::to<std::vector>;

        VkGraphicsPipelineCreateInfo pipeline_info {
            .sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount             = (u32) stages.size(),
            .pStages                = stages.data(),
            .pVertexInputState      = &vertex_input_info,
            .pInputAssemblyState    = &input_assembly_info,
            .pViewportState         = &viewport_info,
            .pRasterizationState    = &rasterization_info,
            .pMultisampleState      = &multisampling_info,
            .pDepthStencilState     = &depth_info,
            .pColorBlendState       = &blending_info,
            .pDynamicState          = &dynamic_state_info,
            .layout                 = *(shader_program->get_layout().uniforms.pipeline_layout),
            .renderPass             = *renderpass,
            .subpass                = 0,
            .basePipelineHandle     = VK_NULL_HANDLE,
            .basePipelineIndex      = -1
        };

        VkPipeline raw_pipeline;
        if (vkCreateGraphicsPipelines(get_context()->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &raw_pipeline) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan graphics pipeline.");
        }

        this->handle = vk_resource<VkPipeline> {
            std::move(raw_pipeline),
            ve::bind<0, 2>(vkDestroyPipeline, get_context()->logical_device.value, nullptr)
        };


        this->settings = settings;
        this->creation_settings = settings;
    }




    void rasterization_pipeline::recreate_framebuffers(void) {
        framebuffers.clear();

        auto attachments = target->get_attachments();
        for (std::size_t i = 0; i < target->get_swap_count(); ++i) {
            auto images = target->get_images_for_index(i);

            std::vector<framebuffer::image_attachment> image_attachments;
            for (const auto& [attachment, image] : views::zip(attachments, images)) {
                image_attachments.push_back(framebuffer::image_attachment { attachment, image });
            }

            framebuffers.push_back(framebuffer { std::move(image_attachments), *renderpass });
        }
    }
}