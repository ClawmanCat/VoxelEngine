#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/image.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/render_target.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/render_context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/command_buffer.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>

#include <vulkan/vulkan.hpp>



namespace ve::gfx {
    class window;
}


namespace ve::gfx::vulkan {
    struct swapchain_member {
        non_owning_image color_image;
        image depth_image;
    };

    struct frame_in_flight {
        vk_resource<VkSemaphore> available, rendered;
        vk_resource<VkFence> submitted;

        command_buffer cmd_buffer;
    };


    struct pre_image_presented_event  { const swapchain_member* img; const frame_in_flight* frame; std::size_t img_index; std::size_t frame_index; };
    struct post_image_presented_event { const swapchain_member* img; const frame_in_flight* frame; std::size_t img_index; std::size_t frame_index; };


    class canvas : public render_target {
    public:
        canvas(window* window, present_mode mode);
        ve_immovable(canvas);


        void rebuild(void);
        void begin_frame(void);
        void end_frame(void);


        std::vector<pipeline_attachment> get_attachments(void) const override;
        std::size_t get_swap_count(void) const override;
        std::size_t get_current_swap_index(void) const override;
        std::vector<image*> get_images_for_index(std::size_t index) override;
        render_context bind(void) override;


        void set_mode(present_mode mode) {
            requires_rebuild |= (mode != this->mode);
            this->mode = mode;
        }


        VE_GET_VAL(owner);
        VE_GET_VAL(mode);
        VE_GET_CREF(queues);
        VE_GET_CREF(surface);
        VE_GET_CREF(swapchain);
        VE_GET_CREF(capabilities);
        VE_GET_CREF(surface_format);
        VE_GET_CREF(creation_info);
        VE_GET_CREF(swapchain_members);
    private:
        window* owner = nullptr;
        window_queues queues;

        vk_resource<VkSurfaceKHR> surface;
        vk_resource<VkSwapchainKHR> swapchain;

        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surface_format;
        VkSwapchainCreateInfoKHR creation_info;
        present_mode mode;

        std::vector<swapchain_member> swapchain_members;
        std::size_t current_image;
        bool requires_rebuild = false;

        // Note: the amount of frames in flight is set to be equal to the number of swapchain images.
        std::vector<frame_in_flight> frames;
        std::size_t current_frame = 0;

    };
}