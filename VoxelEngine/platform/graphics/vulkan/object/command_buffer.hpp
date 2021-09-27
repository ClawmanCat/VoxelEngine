#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class canvas;


    class command_buffer {
    public:
        command_buffer(void) = default;
        ve_swap_move_only(command_buffer, handle, target, wait_semaphore, signal_semaphore, fence, wait_stages, queue_owner, was_submitted);


        explicit command_buffer(queue_family::enum_t target, const canvas* queue_owner = nullptr) : target(target), queue_owner(queue_owner) {
            VkCommandBufferAllocateInfo allocation_info {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = get_queue(target).command_pool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };

            if (vkAllocateCommandBuffers(get_context()->logical_device, &allocation_info, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to allocate command buffer.");
            }

            begin_recording();
        }


        ~command_buffer(void) {
            if (handle) {
                submit();

                vkDeviceWaitIdle(get_context()->logical_device);
                vkFreeCommandBuffers(get_context()->logical_device, get_target_queue().command_pool, 1, &handle);
            }
        }


        void record(auto command, auto&&... args) {
            if (std::exchange(was_submitted, false)) {
                vkResetCommandBuffer(handle, 0);
                begin_recording();
            }

            command(handle, fwd(args)...);
        }


        void submit(void) {
            if (was_submitted) return;

            vkEndCommandBuffer(handle);

            VkSubmitInfo submit_info {
                .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount   = u32(wait_semaphore != VK_NULL_HANDLE),
                .pWaitSemaphores      = &wait_semaphore,
                .pWaitDstStageMask    = &wait_stages,
                .commandBufferCount   = 1,
                .pCommandBuffers      = &handle,
                .signalSemaphoreCount = u32(signal_semaphore != VK_NULL_HANDLE),
                .pSignalSemaphores    = &signal_semaphore
            };

            vkQueueSubmit(get_target_queue().queue, 1, &submit_info, fence);
            was_submitted = true;
        }


        void dont_submit_before_destruction(void) {
            was_submitted = true;
        }


        VE_GET_CREF(handle);
        VE_GET_CREF(target);
    private:
        VkCommandBuffer handle = VK_NULL_HANDLE;
        queue_family::enum_t target;

        VkSemaphore wait_semaphore = VK_NULL_HANDLE;
        VkSemaphore signal_semaphore = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        const canvas* queue_owner;
        bool was_submitted = false;


        void begin_recording(void) {
            VkCommandBufferBeginInfo begin_info {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            vkBeginCommandBuffer(handle, &begin_info);
        }


        const queue_data& get_target_queue(void) {
            return queue_owner ? get_queue(target, *queue_owner) : get_queue(target);
        }

    public:
        VE_GET_SET_VAL(wait_semaphore);
        VE_GET_SET_VAL(signal_semaphore);
        VE_GET_SET_VAL(fence);
        VE_GET_SET_VAL(wait_stages);
    };
}