#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class command_buffer {
    public:
        command_buffer(void) = default;
        ve_swap_move_only(command_buffer, handle);


        explicit command_buffer(queue_family::enum_t target) : target(target) {
            VkCommandBufferAllocateInfo allocation_info {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = get_queue(target).command_pool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };

            if (vkAllocateCommandBuffers(get_context()->logical_device, &allocation_info, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to allocate command buffer.");
            }


            VkCommandBufferBeginInfo begin_info {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            vkBeginCommandBuffer(handle, &begin_info);
        }


        ~command_buffer(void) {
            if (handle) {
                auto& target_queue = get_queue(target);


                vkEndCommandBuffer(handle);

                VkSubmitInfo submit_info {
                    .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers    = &handle
                };

                vkQueueSubmit(target_queue.queue, 1, &submit_info, VK_NULL_HANDLE);
                vkQueueWaitIdle(target_queue.queue);


                vkFreeCommandBuffers(get_context()->logical_device, target_queue.command_pool, 1, &handle);
            }
        }


        void record(auto command, auto&&... args) {
            command(handle, fwd(args)...);
        }


        VE_GET_CREF(handle);
        VE_GET_CREF(target);
    private:
        VkCommandBuffer handle = VK_NULL_HANDLE;
        queue_family::enum_t target;
    };
}