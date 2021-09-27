#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context_helpers.hpp>
#include <VoxelEngine/platform/graphics/vulkan/presentation/canvas.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/utility/vector.hpp>


namespace ve::gfx::vulkan {
    global_queues create_global_queues(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device) {
        const auto indices = get_global_queue_families(physical_device);

        return create_filled_array<queue_family::num_global_families>([&] (std::size_t i) {
            return queue_data {
                .queue        = detail::get_queue(logical_device, indices[i]),
                .command_pool = detail::create_command_pool(logical_device, indices[i]),
                .family_index = indices[i]
            };
        });
    }


    window_queues create_window_queues(VkInstance instance, VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface) {
        const auto indices = get_window_queue_families(physical_device, surface);

        return create_filled_array<queue_family::num_window_families>([&] (std::size_t i) {
            return queue_data {
                .queue        = detail::get_queue(logical_device, indices[i]),
                .command_pool = detail::create_command_pool(logical_device, indices[i]),
                .family_index = indices[i]
            };
        });
    }


    std::vector<u32> get_global_queue_families(VkPhysicalDevice device) {
        std::vector<u32> result;

        auto checked_insert = [&, i = (u32) queue_family::first_global_family] (auto family, auto pred) mutable {
            // This assert will fail if the queue family enum is changed and this function hasn't been updated.
            VE_DEBUG_ASSERT(
                family == magic_enum::enum_value<queue_family::enum_t>(i++),
                "Missing initializer for queue family ", magic_enum::enum_name(family)
            );

            result.push_back(detail::get_queue_family(device, pred));
        };


        checked_insert(queue_family::GRAPHICS_QUEUE, detail::is_graphics_queue);
        checked_insert(queue_family::COMPUTE_QUEUE,  detail::is_compute_queue);

        return result;
    }


    std::vector<u32> get_window_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
        std::vector<u32> result;

        auto checked_insert = [&, i = (u32) queue_family::first_window_family] (auto family, auto pred) mutable {
            // This assert will fail if the queue family enum is changed and this function hasn't been updated.
            VE_DEBUG_ASSERT(
                family == magic_enum::enum_value<queue_family::enum_t>(i++),
                "Missing initializer for queue family ", magic_enum::enum_name(family)
            );

            result.push_back(detail::get_queue_family(device, pred));
        };


        checked_insert(queue_family::PRESENT_QUEUE, bind<2, 3>(detail::is_present_queue, device, surface));

        return result;
    }


    const queue_data& get_queue(queue_family::enum_t family) {
        VE_DEBUG_ASSERT(queue_family::is_global(family), "To get a window-specific queue, please provide a window.");
        return get_context()->queues[family - queue_family::first_global_family];
    }


    const queue_data& get_queue(queue_family::enum_t family, const canvas& canvas) {
        return queue_family::is_global(family)
           ? get_context()->queues[family - queue_family::first_global_family]
           : canvas.get_queues()[family - queue_family::first_window_family];
    }


    const queue_data& get_queue(queue_family::enum_t family, const window& window) {
        return get_queue(family, *window.get_canvas());
    }
}