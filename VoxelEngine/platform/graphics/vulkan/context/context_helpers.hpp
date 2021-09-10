#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/dependent/game_callbacks.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/debug.hpp>

#include <vulkan/vulkan.hpp>
#include <SDL_video.h>
#include <SDL_vulkan.h>


namespace ve::gfx::vulkan::detail {
    inline vk_resource<VkInstance> create_instance(SDL_Window* window, const vulkan_settings& settings) {
        auto layer_ptrs = settings.validation_layers | views::transform(ve_get_field(c_str())) | ranges::to<std::vector>;


        // Get additional Vulkan extensions required by SDL.
        u32 num_extensions = 0;
        {
            SDL_bool success = SDL_Vulkan_GetInstanceExtensions(window, &num_extensions, nullptr);
            VE_ASSERT(success, "Failed to get Vulkan instance extension count.");
        }

        std::vector<const char*> extensions;
        {
            extensions.resize(num_extensions);
            SDL_bool success = SDL_Vulkan_GetInstanceExtensions(window, &num_extensions, &extensions.front());
            VE_ASSERT(success, "Failed to get Vulkan extensions.");
        }

        // Append manually specified extensions.
        ranges::transform(settings.global_extensions, std::back_inserter(extensions), ve_get_field(c_str()));


        // Assert all required extensions are supported.
        validate_availability<VkExtensionProperties, const char*>(
            extensions,
            [](const auto& extension) { return (const char*) extension.extensionName; },
            vkEnumerateInstanceExtensionProperties,
            nullptr
        );

        // Assert all required validation layers are supported.
        validate_availability<VkLayerProperties, const char*>(
            layer_ptrs,
            [](const auto& layer) { return (const char*) layer.layerName; },
            vkEnumerateInstanceLayerProperties
        );


        auto make_version = [](auto ver) { return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch); };

        VkApplicationInfo app_info {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = game_callbacks::get_info()->display_name.c_str(),
            .applicationVersion = make_version(game_callbacks::get_info()->version),
            .pEngineName        = engine::info.display_name.c_str(),
            .engineVersion      = make_version(engine::info.version),
            .apiVersion         = VK_API_VERSION_1_2
        };


        // Debugging instance creation requires a separate debugger.
        VkDebugUtilsMessengerCreateInfoEXT debug_info = get_debug_messenger_setup_info(debug_messenger_callback);

        VkInstanceCreateInfo instance_info {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = settings.enable_debugger ? &debug_info : nullptr,
            .pApplicationInfo        = &app_info,
            .enabledLayerCount       = (u32) layer_ptrs.size(),
            .ppEnabledLayerNames     = layer_ptrs.empty() ? nullptr : layer_ptrs.data(),
            .enabledExtensionCount   = (u32) extensions.size(),
            .ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data()
        };

        VkInstance instance;
        if (vkCreateInstance(&instance_info, nullptr, &instance) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan instance.");
        }

        return { std::move(instance), bind<1>(vkDestroyInstance, nullptr) };
    }




    inline vk_resource<VkPhysicalDevice> pick_physical_device(VkInstance instance, const VkPhysicalDeviceFeatures& required_features) {
        auto devices = enumerate_simple_vk_property<VkPhysicalDevice>(vkEnumeratePhysicalDevices, instance);

        VkPhysicalDevice selected_device = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures selected_device_features;
        VkPhysicalDeviceProperties selected_device_properties;

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);


            bool has_required_features = true;

            // Using PFR directly would be clearer but this confuses Clang, causing it to spit out nonsensical errors.
            meta::create_pack::from_decomposable<VkPhysicalDeviceFeatures>::foreach_indexed([&] <typename T, std::size_t I> () {
                const auto& required  = boost::pfr::get<I>(required_features);
                const auto& supported = boost::pfr::get<I>(features);

                if constexpr (std::is_same_v<T, VkBool32>) {
                    if (required && !supported) has_required_features = false;
                } else if (std::is_same_v<T, VkDeviceSize>) {
                    if (required > supported) has_required_features = false;
                } else {
                    static_assert(meta::always_false_v<T>, "VkPhysicalDeviceFeatures contains members not accounted for.");
                }
            });

            if (!has_required_features) continue;


            auto assign = [&] {
                selected_device = device;
                selected_device_features = features;
                selected_device_properties = properties;
            };


            // Any device is better than no device.
            if (selected_device == VK_NULL_HANDLE) { assign(); continue; }


            // Dedicated GPUs are preferred over integrated ones.
            if (
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                selected_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            ) { assign(); continue; }


            // Integrated GPUs are preferred over whatever else might be showing up as a Vulkan device.
            if (
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
                selected_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                selected_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
            ) { assign(); continue; }


            // In the case of multiple GPUs, pick the one with more VRAM.
            auto memory_size = [](const auto& device) {
                VkPhysicalDeviceMemoryProperties properties;
                vkGetPhysicalDeviceMemoryProperties(device, &properties);

                std::size_t max_heap_size = 0;
                for (const auto& heap : std::span<VkMemoryHeap>(properties.memoryHeaps, properties.memoryHeaps + properties.memoryHeapCount)) {
                    if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > max_heap_size) max_heap_size = heap.size;
                }

                return max_heap_size;
            };

            if (memory_size(device) > memory_size(selected_device)) { assign(); continue; }
        }


        VE_ASSERT(selected_device != VK_NULL_HANDLE, "No suitable Vulkan device found.");
        return { std::move(selected_device) };
    }




    inline u32 get_queue_family(VkPhysicalDevice device, auto pred) {
        auto families = enumerate_simple_vk_property<VkQueueFamilyProperties>(
            vkGetPhysicalDeviceQueueFamilyProperties,
            device
        );

        for (const auto& [i, family] : families | views::enumerate) {
            if (pred(i, family)) return i;
        }

        VE_ASSERT(false, "Failed to find a queue family matching the required properties.");
        VE_UNREACHABLE;
    }


    inline bool is_graphics_queue(u32 index, VkQueueFamilyProperties properties) {
        return properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    }

    inline bool is_compute_queue(u32 index, VkQueueFamilyProperties properties) {
        return properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
    }

    inline bool is_present_queue(u32 index, VkQueueFamilyProperties properties, VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkBool32 result = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &result);
        return result;
    }




    inline vk_resource<VkDevice> create_logical_device(
        VkPhysicalDevice physical_device,
        VkPhysicalDeviceFeatures features,
        std::vector<u32> queue_families,
        const std::vector<std::string>& device_extensions
    ) {
        VE_ASSERT(!queue_families.empty(), "Cannot create device without associated queue families.");

        // If two or more families are actually the same, (e.g. the graphics family is also the present family)
        // we need to remove duplicate entries.
        queue_families = std::move(queue_families) | actions::unique;


        std::vector<VkDeviceQueueCreateInfo> queue_info;
        constexpr float priority = 1.0f;

        for (const auto& family : queue_families) {
            queue_info.push_back(VkDeviceQueueCreateInfo {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount       = 1,
                .pQueuePriorities = &priority
            });
        }


        auto extension_ptrs = device_extensions | views::transform(ve_get_field(c_str())) | ranges::to<std::vector>;
        VkDeviceCreateInfo device_info {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount    = (u32) queue_info.size(),
            .pQueueCreateInfos       = queue_info.data(),
            .enabledExtensionCount   = (u32) extension_ptrs.size(),
            .ppEnabledExtensionNames = extension_ptrs.data(),
            .pEnabledFeatures        = &features
        };


        VkDevice logical_device;
        if (vkCreateDevice(physical_device, &device_info, nullptr, &logical_device) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create logical Vulkan device.");
        }

        return { std::move(logical_device), bind<1>(vkDestroyDevice, nullptr) };
    }




    inline vk_resource<VkQueue> get_queue(VkDevice device, u32 family) {
        VkQueue result = VK_NULL_HANDLE;

        vkGetDeviceQueue(device, family, 0, &result);
        VE_ASSERT(result != VK_NULL_HANDLE, "Failed to get device queue.");

        return { std::move(result) };
    }




    inline vk_resource<VkCommandPool> create_command_pool(VkDevice device, u32 family) {
        VkCommandPoolCreateInfo info {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = 0,
            .queueFamilyIndex = family
        };

        VkCommandPool pool;
        if (vkCreateCommandPool(device, &info, nullptr, &pool) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan command pool.");
        }

        return { std::move(pool), bind<0, 2>(vkDestroyCommandPool, device, nullptr) };
    }
}