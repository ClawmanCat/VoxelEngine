#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>

#include <vulkan/vulkan.hpp>
#include <magic_enum.hpp>


namespace ve::gfx {
    class window;

    namespace vulkan {
        class canvas;
    }
}


namespace ve::gfx::vulkan {
    // Implicitly castable enum without namespace pollution.
    namespace queue_family {
        enum enum_t : u32 { GRAPHICS_QUEUE = 0, COMPUTE_QUEUE = 1, PRESENT_QUEUE = 2 };

        constexpr inline enum_t first_global_family = GRAPHICS_QUEUE;
        constexpr inline enum_t first_window_family = PRESENT_QUEUE;


        constexpr inline u32 num_global_families = first_window_family - first_global_family;
        constexpr inline u32 num_window_families = ((u32) magic_enum::enum_count<enum_t>()) - first_window_family;

        constexpr bool is_global(enum_t family)          { return family <  first_window_family; }
        constexpr bool is_window_specific(enum_t family) { return family >= first_window_family; }
    }


    struct queue_data {
        vk_resource<VkQueue> queue;
        vk_resource<VkCommandPool> command_pool;
        u32 family_index;
    };

    using global_queues = std::array<queue_data, queue_family::num_global_families>;
    using window_queues = std::array<queue_data, queue_family::num_window_families>;


    extern global_queues create_global_queues(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device);
    extern window_queues create_window_queues(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface);

    extern std::vector<u32> get_global_queue_families(VkPhysicalDevice device);
    extern std::vector<u32> get_window_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);


    extern const queue_data& get_queue(queue_family::enum_t family);
    extern const queue_data& get_queue(queue_family::enum_t family, const canvas& canvas);
    extern const queue_data& get_queue(queue_family::enum_t family, const window& window);
}