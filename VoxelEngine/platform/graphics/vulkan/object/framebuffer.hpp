#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/image.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class framebuffer {
    public:
        framebuffer(void) = default;

        explicit framebuffer(unique<image>&& img, VkImageLayout target_layout) : target_layout(target_layout), img(std::move(img)) {
            VE_NOT_YET_IMPLEMENTED;
        }

        ~framebuffer(void) {
            if (handle) vkDestroyFramebuffer(get_context()->logical_device, handle, nullptr);
        }

        ve_swap_move_only(framebuffer, handle, target_layout, img);


        void on_rendered(void) {
            img->transition_layout_external(target_layout);
        }


        const image& get_image(void) const { return *img; }
        VE_GET_CREF(handle);
        VE_GET_VAL(target_layout);
    private:
        VkFramebuffer handle = VK_NULL_HANDLE;
        VkImageLayout target_layout;
        unique<image> img;
    };
}