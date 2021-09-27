#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/image.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class framebuffer {
    public:
        struct image_attachment {
            pipeline_attachment attachment;
            image* image;
        };


        framebuffer(void) = default;

        framebuffer(std::vector<image_attachment> attachments, VkRenderPass renderpass) : attachments(std::move(attachments)) {
            attachment_views = this->attachments | views::transform(ve_get_field(image->create_view())) | ranges::to<std::vector>;


            // Vulkan expects pointers to the view directly rather than the associated RAII object.
            auto view_pointers = attachment_views | views::transform(ve_get_field(value)) | ranges::to<std::vector>;

            VkFramebufferCreateInfo info {
                .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass      = renderpass,
                .attachmentCount = (u32) view_pointers.size(),
                .pAttachments    = view_pointers.data(),
                // All images are the same size, so we can query any one of them.
                .width           = this->attachments[0].image->get_size().x,
                .height          = this->attachments[0].image->get_size().y,
                .layers          = 1
            };

            if (vkCreateFramebuffer(get_context()->logical_device, &info, nullptr, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to create Vulkan framebuffer.");
            }
        }

        ~framebuffer(void) {
            if (handle != VK_NULL_HANDLE) vkDestroyFramebuffer(get_context()->logical_device, handle, nullptr);
        }

        ve_rt_swap_move_only(framebuffer, handle, attachments, attachment_views);


        VE_GET_CREF(handle);
        VE_GET_CREF(attachments);
        VE_GET_CREF(attachment_views);
    private:
        VkFramebuffer handle = VK_NULL_HANDLE;

        std::vector<image_attachment> attachments;
        std::vector<image_view> attachment_views;
    };
}