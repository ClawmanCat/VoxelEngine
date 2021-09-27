#include <VoxelEngine/platform/graphics/vulkan/presentation/canvas.hpp>
#include <VoxelEngine/platform/graphics/vulkan/presentation/window_helpers.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/utility/functional.hpp>

#include <SDL_video.h>
#include <SDL_vulkan.h>


namespace ve::gfx::vulkan {
    canvas::canvas(window* window, present_mode mode) : owner(window), mode(mode) {
        VkSurfaceKHR raw_surface;
        if (!SDL_Vulkan_CreateSurface(window->get_handle(), get_context()->instance, &raw_surface)) {
            VE_ASSERT(false, "Failed to create Vulkan surface.");
        }

        this->surface = vk_resource<VkSurfaceKHR> {
            std::move(raw_surface),
            ve::bind<0, 2>(vkDestroySurfaceKHR, *(get_context()->instance), nullptr)
        };


        queues = create_window_queues(
            get_context()->instance,
            get_context()->physical_device,
            get_context()->logical_device,
            surface
        );


        auto surface_formats = detail::enumerate_simple_vk_property<VkSurfaceFormatKHR>(
            vkGetPhysicalDeviceSurfaceFormatsKHR,
            get_context()->physical_device,
            *surface
        );

        VE_ASSERT(!surface_formats.empty(), "No supported surface formats found for swapchain.");
        surface_format = get_context()->settings.pick_swapchain_surface_format(surface_formats);


        rebuild();
    }


    void canvas::rebuild(void) {
        vkDeviceWaitIdle(get_context()->logical_device);

        // Clear old swap chain.
        swapchain_members.clear();

        for (auto& frame : frames) frame.cmd_buffer.dont_submit_before_destruction();
        frames.clear();

        current_frame = 0;


        // Some parts of the capabilities (e.g. size) may change and should be re-queried.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(get_context()->physical_device, *surface, &capabilities);


        vec2ui extent = (capabilities.currentExtent.width == max_value<u32> || capabilities.currentExtent.height == max_value<u32>)
            ? owner->get_canvas_size()
            : vec2ui { capabilities.currentExtent.width, capabilities.currentExtent.height };

        extent.x = std::clamp(extent.x, capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
        extent.y = std::clamp(extent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);


        u32 min_image_count = 0, desired_image_count = 0;
        u32 max_image_count = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : max_value<u32>;

        switch (mode) {
            case present_mode::IMMEDIATE:       min_image_count = 1; desired_image_count = 2; break;
            case present_mode::VSYNC:           min_image_count = 1; desired_image_count = 2; break;
            case present_mode::TRIPLE_BUFFERED: min_image_count = 3; desired_image_count = 3; break;
        }

        // + 1 because https://vulkan.lunarg.com/doc/view/1.2.182.0/windows/1.2-extensions/vkspec.html#VUID-vkAcquireNextImageKHR-swapchain-01802
        VE_ASSERT(
            capabilities.minImageCount + 1 <= max_image_count,
            "Swapchain must support at least one more image than the minimum value."
        );

        desired_image_count = std::clamp(desired_image_count, capabilities.minImageCount + 1, max_image_count);

        VE_ASSERT(
            desired_image_count >= min_image_count,
            "Swapchain does not support the creation of enough images for present mode ", magic_enum::enum_name(mode), "."
        );


        u32 queue_families[2] = {
            get_queue(queue_family::GRAPHICS_QUEUE, *this).family_index,
            get_queue(queue_family::PRESENT_QUEUE, *this).family_index
        };

        bool multi_queue = (queue_families[0] != queue_families[1]);


        VkSwapchainCreateInfoKHR swapchain_info {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface               = *surface,
            .minImageCount         = desired_image_count,
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
            .oldSwapchain          = swapchain
        };

        VkSwapchainKHR new_handle;
        if (vkCreateSwapchainKHR(get_context()->logical_device, &swapchain_info, nullptr, &new_handle) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan swapchain.");
        }

        swapchain.store(std::move(new_handle));


        auto images = detail::enumerate_simple_vk_property<VkImage>(
            vkGetSwapchainImagesKHR,
            get_context()->logical_device,
            *swapchain
        );


        auto depth_fmt_supported = [](const auto& kv) -> bool {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(get_context()->physical_device, kv.first, &properties);

            return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        };

        std::vector<VkFormat> depth_formats = image_formats
            | views::filter([](const auto& kv) { return kv.second.aspects & VK_IMAGE_ASPECT_DEPTH_BIT; })
            | views::filter(depth_fmt_supported)
            | views::keys
            | ranges::to<std::vector>;

        VkFormat depth_format = get_context()->settings.pick_swapchain_depth_format(depth_formats);


        for (auto&& img : images) {
            non_owning_image color_image {
                std::move(img),
                image::arguments {
                    .size         = { swapchain_info.imageExtent.width, swapchain_info.imageExtent.height },
                    .usage        = swapchain_info.imageUsage,
                    // Layout will transition when image is presented.
                    .layout       = VK_IMAGE_LAYOUT_UNDEFINED,
                    .pixel_format = swapchain_info.imageFormat
                }
            };

            image depth_image { image::arguments {
                .size           = { swapchain_info.imageExtent.width, swapchain_info.imageExtent.height },
                .usage          = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .layout         = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .pixel_format   = depth_format
            }};


            swapchain_members.push_back(swapchain_member {
                .color_image = std::move(color_image),
                .depth_image = std::move(depth_image)
            });

            auto& frame = frames.emplace_back(frame_in_flight {
                .available   = detail::create_semaphore(),
                .rendered    = detail::create_semaphore(),
                .submitted   = detail::create_fence(),
                .cmd_buffer  = command_buffer { queue_family::GRAPHICS_QUEUE, this }
            });

            frame.cmd_buffer.set_wait_semaphore(frame.available);
            frame.cmd_buffer.set_signal_semaphore(frame.rendered);
            frame.cmd_buffer.set_fence(frame.submitted);
            frame.cmd_buffer.set_wait_stages(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }


        dispatch_event(target_invalidated_event { });
    }


    void canvas::begin_frame(void) {
        // Rebuild if the present mode changed.
        if (std::exchange(requires_rebuild, false)) rebuild();


        // Attempt to acquire the next swapchain image.
        acquire_image:
        u32 image_index;
        auto result = vkAcquireNextImageKHR(get_context()->logical_device, swapchain, max_value<u64>, frames[current_frame].available, VK_NULL_HANDLE, &image_index);


        // If acquisition failed, rebuild and try again.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            rebuild();
            goto acquire_image;
        } else if (result != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to acquire swapchain image.");
        }


        current_image = (std::size_t) image_index;

        dispatch_event(pre_image_presented_event {
            .img   = &swapchain_members[current_image],
            .frame = &frames[current_frame]
        });
    }


    void canvas::end_frame(void) {
        frames[current_frame].cmd_buffer.submit();


        u32 image_index = (u32) current_image;
        VkPresentInfoKHR present_info {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &frames[current_frame].rendered.value,
            .swapchainCount     = 1,
            .pSwapchains        = &swapchain.value,
            .pImageIndices      = &image_index
        };

        vkQueuePresentKHR(get_queue(queue_family::PRESENT_QUEUE, *this).queue, &present_info);

        vkWaitForFences(get_context()->logical_device, 1, &frames[current_frame].submitted.value, VK_TRUE, max_value<u64>);
        vkResetFences(get_context()->logical_device, 1, &frames[current_frame].submitted.value);


        dispatch_event(post_image_presented_event {
            .img   = &swapchain_members[current_image],
            .frame = &frames[current_frame]
        });


        current_frame = (current_frame + 1) % frames.size();
    }


    std::vector<pipeline_attachment> canvas::get_attachments(void) const {
        pipeline_attachment color { "color", pipeline_attachment::PRESENTABLE };
        color.set_named(true);

        pipeline_attachment depth { std::nullopt, pipeline_attachment::DEPTH_BUFFER };
        depth.set_named(false);

        return { std::move(color), std::move(depth) };
    }


    std::size_t canvas::get_swap_count(void) const {
        return swapchain_members.size();
    }


    std::size_t canvas::get_current_swap_index(void) const {
        return current_image;
    }


    std::vector<image*> canvas::get_images_for_index(std::size_t index) {
        return {
            &swapchain_members.at(index).color_image,
            &swapchain_members.at(index).depth_image
        };
    }


    render_context canvas::bind(void) {
        return render_context {
            this,
            &frames[current_frame].cmd_buffer
        };
    }
}