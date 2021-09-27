#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class pipeline_attachment {
    public:
        // If no name is provided, attachments are resolved by index,
        // e.g. the first output of the fragment shader will be matched against the first pipeline_attachment
        // passed to create_pipeline.
        // This is probably the desired option for most cases, as it matches OpenGLs behaviour.
        using maybe_string = std::optional<std::string>;


        enum usage : std::underlying_type_t<VkImageLayout> {
            COLOR_BUFFER = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            DEPTH_BUFFER = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            PRESENTABLE  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            INPUT        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        static VkImageLayout intermediate_layout_for_usage(enum usage usage) {
            return (VkImageLayout) (usage == PRESENTABLE ? COLOR_BUFFER : usage);
        }

        static VkImageLayout start_layout_for_usage(enum usage usage) {
            return usage == PRESENTABLE ? VK_IMAGE_LAYOUT_UNDEFINED : (VkImageLayout) usage;
        }


        explicit pipeline_attachment(maybe_string name = std::nullopt, enum usage usage = COLOR_BUFFER) :
            name(std::move(name)),
            usage(usage),
            start_layout(start_layout_for_usage(usage)),
            intermediate_layout(intermediate_layout_for_usage(usage)),
            target_layout((VkImageLayout) usage)
        {}


        pipeline_attachment(maybe_string name, VkImageLayout start, VkImageLayout intermediate, VkImageLayout target, enum usage usage) :
            name(std::move(name)),
            usage(usage),
            start_layout(start),
            intermediate_layout(intermediate),
            target_layout(target)
        {}

    private:
        maybe_string name;
        enum usage usage;

        // The intermediate layout is the layout the framebuffer will have while being rendered to,
        // the target layout is the layout it will be transitioned to after rendering.
        // These will be different, e.g. in the case of a swapchain image, where we need to render to it as a color buffer,
        // but then need to transition it to make it presentable.
        VkImageLayout start_layout;
        VkImageLayout intermediate_layout;
        VkImageLayout target_layout;

        VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;

        // Attachments are unnamed if they are not explicitly mentioned as an output of the final shader stage, e.g. the depth buffer.
        bool named = true;
    public:
        VE_GET_SET_CREF(name);
        VE_GET_SET_VAL(usage);
        VE_GET_SET_VAL(start_layout);
        VE_GET_SET_VAL(intermediate_layout);
        VE_GET_SET_VAL(target_layout);
        VE_GET_SET_BOOL_IS(named);

        // It is rare that these should be changed, so don't bother parameterizing the constructor on them.
        VE_GET_SET_VAL(load_op);
        VE_GET_SET_VAL(store_op);
    };
}