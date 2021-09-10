#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    class device_memory {
    public:
        constexpr static inline VkMemoryPropertyFlags gpu_memory = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        constexpr static inline VkMemoryPropertyFlags cpu_memory = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;


        device_memory(void) = default;
        ve_swap_move_only(device_memory, handle);


        explicit device_memory(
            VkMemoryRequirements requirements,
            VkMemoryPropertyFlags properties = gpu_memory
        ) {
            VkMemoryAllocateInfo allocate_info {
                .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize  = requirements.size,
                .memoryTypeIndex = get_memory_type(requirements.memoryTypeBits, properties)
            };

            if (vkAllocateMemory(get_context()->logical_device, &allocate_info, nullptr, &handle) != VK_SUCCESS) {
                VE_ASSERT(false, "Failed to allocate memory from device.");
            }
        }


        ~device_memory(void) {
            if (handle != VK_NULL_HANDLE) vkFreeMemory(get_context()->logical_device, handle, nullptr);
        }


        VE_GET_CREF(handle);
        VE_GET_CREF(properties);
        VE_GET_VAL(size);
    private:
        VkDeviceMemory handle = VK_NULL_HANDLE;
        VkMemoryPropertyFlags properties = 0;

        u32 size = 0;


        // Get the first memory type matching the requirements in 'flags'.
        // 'filter' is used to black/whitelist memory types at certain indices.
        static u32 get_memory_type(u32 filter, const VkMemoryPropertyFlags& flags) {
            VkPhysicalDeviceMemoryProperties properties;
            vkGetPhysicalDeviceMemoryProperties(get_context()->physical_device, &properties);

            for (u32 i = 0; i < properties.memoryTypeCount; ++i) {
                if (filter & (1 << i) && (properties.memoryTypes[i].propertyFlags & flags) == flags) return i;
            }

            VE_ASSERT(false, "No memory type matching the provided requirements is available.");
            VE_UNREACHABLE;
        }
    };
}