#include <VoxelEngine/platform/graphics/vulkan/presentation/swapchain.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve::gfx::vulkan {
    swapchain::swapchain(window* owner, present_mode mode) : owner(owner) {
        // Select pixel format and color space for swapchain.
        auto surface_formats = detail::enumerate_simple_vk_property<VkSurfaceFormatKHR>(
            vkGetPhysicalDeviceSurfaceFormatsKHR,
            get_context()->physical_device,
            owner->get_canvas()->get_handle()
        );

        VE_ASSERT(!surface_formats.empty(), "No supported surface formats found for swapchain.");
        surface_format = get_context()->settings.pick_swapchain_surface_format(surface_formats);


        rebuild(mode);
    }


    swapchain::~swapchain(void) {
        if (handle != VK_NULL_HANDLE) vkDestroySwapchainKHR(get_context()->logical_device, handle, nullptr);
    }


    void swapchain::rebuild(present_mode mode) {
        dispatch_event(pre_swapchain_rebuild_event { });


        // Clear old swap chain.
        swapchain_members.clear();
        current_member = 0;

        if (handle != VK_NULL_HANDLE) vkDestroySwapchainKHR(get_context()->logical_device, handle, nullptr);


        // Some parts of the capabilities (e.g. size) may change and should be re-queried.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(get_context()->physical_device, owner->get_canvas()->get_handle(), &capabilities);


        vec2ui extent = (capabilities.currentExtent.width == max_value<u32> || capabilities.currentExtent.height == max_value<u32>)
            ? owner->get_canvas_size()
            : vec2ui { capabilities.currentExtent.width, capabilities.currentExtent.height };

        extent.x = std::clamp(extent.x, capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
        extent.y = std::clamp(extent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);


        u32 image_count = 0;
        switch (mode) {
            case present_mode::IMMEDIATE:       image_count = 1; break;
            case present_mode::DOUBLE_BUFFERED: image_count = 2; break;
            case present_mode::TRIPLE_BUFFERED: image_count = 3; break;
        }

        u32 max_image_count = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : max_value<u32>;
        image_count = std::clamp(image_count, capabilities.minImageCount, max_image_count);


        u32 queue_families[2] = {
            get_queue(queue_family::GRAPHICS_QUEUE).family_index,
            get_queue(queue_family::PRESENT_QUEUE).family_index
        };

        bool multi_queue = (queue_families[0] != queue_families[1]);


        VkSwapchainCreateInfoKHR swapchain_info {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface               = owner->get_canvas()->get_handle(),
            .minImageCount         = image_count,
            .imageFormat           = surface_format.format,
            .imageColorSpace       = surface_format.colorSpace,
            .imageExtent           = VkExtent2D { extent.x, extent.y },
            .imageArrayLayers      = 1,
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageSharingMode      = multi_queue ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = multi_queue ? 2u : 0u,
            .pQueueFamilyIndices   = multi_queue ? queue_families : nullptr,
            .preTransform          = capabilities.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = present_mode_ve_to_vk(mode),
            .clipped               = VK_TRUE,
            .oldSwapchain          = handle
        };

        if (vkCreateSwapchainKHR(get_context()->logical_device, &swapchain_info, nullptr, &handle) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan swapchain.");
        }


        auto images = detail::enumerate_simple_vk_property<VkImage>(
            vkGetSwapchainImagesKHR,
            get_context()->logical_device,
            handle
        );


        for (auto&& img : images) {
            auto color_image = make_unique<non_owning_image>(
                std::move(img),
                image::arguments {
                    .size         = { swapchain_info.imageExtent.width, swapchain_info.imageExtent.height },
                    .usage        = swapchain_info.imageUsage,
                    // Layout will transition when image is presented.
                    .layout       = VK_IMAGE_LAYOUT_UNDEFINED,
                    .pixel_format = swapchain_info.imageFormat
                }
            );

            auto depth_image = make_unique<image>(image::arguments {
                .size           = { swapchain_info.imageExtent.width, swapchain_info.imageExtent.height },
                .usage          = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .layout         = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .pixel_format   = VK_FORMAT_D32_SFLOAT
            });


            swapchain_members.push_back(swapchain_member {
                .color_framebuffer = framebuffer { std::move(color_image), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
                .depth_framebuffer = framebuffer { std::move(depth_image), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL }
            });
        }


        dispatch_event(post_swapchain_rebuild_event { });
    }


    void swapchain::swap(void) {
        dispatch_event(pre_image_presented_event { &swapchain_members[current_member], current_member });

        // TODO: Present image!
        // 1. Check out of date / suboptimal / resized and rebuild, then skip
        // 2. Skip if minimized
        // 3. Render otherwise

        dispatch_event(post_image_presented_event { &swapchain_members[current_member], current_member });
        current_member = (current_member + 1) % swapchain_members.size();
    }
}