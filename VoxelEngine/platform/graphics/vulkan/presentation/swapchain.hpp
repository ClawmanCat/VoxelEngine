#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/image.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/framebuffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/presentation/window_helpers.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx {
    class window;
}


namespace ve::gfx::vulkan {
    struct swapchain_member {
        framebuffer color_framebuffer;
        framebuffer depth_framebuffer;
    };


    struct pre_image_presented_event  { const swapchain_member* img; std::size_t index; };
    struct post_image_presented_event { const swapchain_member* img; std::size_t index; };

    struct pre_swapchain_rebuild_event  { };
    struct post_swapchain_rebuild_event { };


    class swapchain : public subscribe_only_view<simple_event_dispatcher<false, u16, true>> {
    public:
        swapchain(window* owner, present_mode mode);
        ~swapchain(void);
        ve_immovable(swapchain);


        void rebuild(present_mode mode);
        void swap(void);


        VE_GET_CREF(creation_info);
        VE_GET_CREF(capabilities);
        VE_GET_CREF(surface_format);
        VE_GET_CREF(handle);
        VE_GET_CREF(swapchain_members);
        VE_GET_VAL(current_member);
        VE_GET_VAL(owner);
    private:
        window* owner;

        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surface_format;

        VkSwapchainCreateInfoKHR creation_info;
        VkSwapchainKHR handle = VK_NULL_HANDLE;

        std::vector<swapchain_member> swapchain_members;
        std::size_t current_member = 0;
    };
}