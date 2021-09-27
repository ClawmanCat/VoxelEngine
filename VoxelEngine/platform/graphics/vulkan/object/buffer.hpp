#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/memory.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/command_buffer.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class buffer {
    public:
        constexpr static inline VkBufferUsageFlags usage_transferable = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;


        buffer(void) = default;
        ve_swap_move_only(buffer, handle, memory, usage, size);


        buffer(
            u32 size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties = device_memory::gpu_memory,
            // Could be part of usage flags, but will be true most of the time, since it is required for the read / write operations.
            bool allow_transfer = true,
            bool shared = false
        ) :
            usage(usage), size(size)
        {
            if (allow_transfer) usage |= buffer::usage_transferable;

            VkBufferCreateInfo info {
                .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size        = size,
                .usage       = usage,
                .sharingMode = shared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE
            };

            if (vkCreateBuffer(get_context()->logical_device, &info, nullptr, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to create Vulkan buffer.");
            }


            VkMemoryRequirements requirements;
            vkGetBufferMemoryRequirements(get_context()->logical_device, handle, &requirements);

            memory = device_memory(requirements, properties);
            vkBindBufferMemory(get_context()->logical_device, handle, memory.get_handle(), 0);
        }


        ~buffer(void) {
            if (handle != VK_NULL_HANDLE) vkDestroyBuffer(get_context()->logical_device, handle, nullptr);
        }


        std::vector<u8> read(u32 count = max_value<u32>, u32 offset = 0) const {
            if (count == max_value<u32>) count = size;


            if (memory.get_properties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Memory can be directly accessed from CPU, no staging buffer required.
                void* source_memory;
                vkMapMemory(get_context()->logical_device, memory.get_handle(), offset, count, 0, &source_memory);


                if (!(memory.get_properties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) [[unlikely]] {
                    VkMappedMemoryRange range {
                        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                        .pNext  = nullptr,
                        .memory = memory.get_handle(),
                        .offset = offset,
                        .size   = count
                    };

                    vkInvalidateMappedMemoryRanges(get_context()->logical_device, 1, &range);
                }


                std::vector<u8> result;
                result.resize(count);

                memcpy(result.data(), source_memory, count);

                vkUnmapMemory(get_context()->logical_device, memory.get_handle());
                return result;
            } else {
                // Data must be download to GPU first using staging buffer.
                buffer staging_buffer { count, VK_BUFFER_USAGE_TRANSFER_DST_BIT, device_memory::cpu_memory };

                copy_to(staging_buffer, count, offset, 0);
                return staging_buffer.read(count, 0);
            }
        }


        void write(std::span<const u8> data, u32 offset = 0) {
            if (memory.get_properties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                // Memory can be directly accessed from CPU, no staging buffer required.
                void* target_memory;

                vkMapMemory(get_context()->logical_device, memory.get_handle(), offset, (u32) data.size(), 0, &target_memory);
                memcpy(target_memory, data.data(), data.size());

                if (!(memory.get_properties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) [[unlikely]] {
                    VkMappedMemoryRange range {
                        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                        .pNext  = nullptr,
                        .memory = memory.get_handle(),
                        .offset = offset,
                        .size   = (u32) data.size()
                    };

                    vkFlushMappedMemoryRanges(get_context()->logical_device, 1, &range);
                }

                vkUnmapMemory(get_context()->logical_device, memory.get_handle());
            } else {
                // Data must be uploaded to GPU first using staging buffer.
                buffer staging_buffer { (u32) data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, device_memory::cpu_memory };

                staging_buffer.write(data);
                staging_buffer.copy_to(*this, data.size(), 0, offset);
            }
        }


        void copy_to(buffer& target, u32 count = max_value<u32>, u32 read_offset = 0, u32 write_offset = 0) const {
            if (count == max_value<u32>) count = size - read_offset;


            command_buffer cb { queue_family::GRAPHICS_QUEUE };

            VkBufferCopy region { .srcOffset = read_offset, .dstOffset = write_offset, .size = count };
            cb.record(vkCmdCopyBuffer, handle, target.handle, 1, &region);
        }


        bool has_storage(void) const { return handle != VK_NULL_HANDLE; }


        VE_GET_CREF(handle);
        VE_GET_CREF(memory);
        VE_GET_CREF(usage);
        VE_GET_VAL(size);
    private:
        VkBuffer handle = VK_NULL_HANDLE;
        device_memory memory;
        VkBufferUsageFlags usage;

        // Note: buffer size is not required to be equal to the size of the underlying memory and must be stored separately.
        u32 size = 0;
    };
}