#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/platform/graphics/vulkan/resource.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/math.hpp>

#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <boost/pfr.hpp>

#include <sstream>
#include <vector>
#include <algorithm>


namespace ve::detail::vk_helpers {
    using namespace ve::graphics;
    
    
    // Loads the function pointer for the given Vulkan function dynamically.
    #define VE_IMPL_LOAD_VKPTR(instance, name)                                      \
    [&]() {                                                                         \
        auto ptr = (PFN_##name) vkGetInstanceProcAddr(instance, #name)              \
        VE_ASSERT(ptr, "Failed to load " #name " method.");                         \
        return ptr;                                                                 \
    }()
    
    
    template <typename T> inline std::string to_string(const T& v) {
        std::stringstream s;
        s << v;
        return s.str();
    }
    
    
    #define VE_IMPL_VK_SWITCH_CASE(out, entry) \
    case entry: out = #entry; break;
    
    #define VE_IMPL_VK_SWITCH_CASE_SET_LVL(level_out, level_val, out, entry) \
    case entry: out = #entry; level_out = logger::level::level_val; break;
    
    
    inline VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_error_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagBitsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* user_data
    ) {
        std::string_view type_str;
        switch (type) {
            VE_IMPL_VK_SWITCH_CASE(type_str, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
            VE_IMPL_VK_SWITCH_CASE(type_str, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            VE_IMPL_VK_SWITCH_CASE(type_str, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            default: break;
        }
    
        type_str.remove_prefix("VK_DEBUG_UTILS_MESSAGE_TYPE_"sv.length());
        type_str.remove_suffix("_BIT_EXT"sv.length());
    
    
        std::string_view severity_str;
        logger::level level;
        switch (severity) {
            VE_IMPL_VK_SWITCH_CASE_SET_LVL(level, DEBUG,   severity_str, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            VE_IMPL_VK_SWITCH_CASE_SET_LVL(level, INFO,    severity_str, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            VE_IMPL_VK_SWITCH_CASE_SET_LVL(level, WARNING, severity_str, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            VE_IMPL_VK_SWITCH_CASE_SET_LVL(level, ERROR,   severity_str, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            default: break;
        }
    
        severity_str.remove_prefix("VK_DEBUG_UTILS_MESSAGE_SEVERITY_"sv.length());
        severity_str.remove_suffix("_BIT_EXT"sv.length());
    
        
        loggers::ve_logger.message(
            "Vulkan "s + type_str + " event with severity " + severity_str + " caught: " + data->pMessage,
            level
        );
        
        
        return VK_FALSE;
    }
    
    
    template <typename Property>
    inline std::vector<Property> enumerate_vk_property(auto get_count, auto get_details) {
        u32 count = 0;
        get_count(&count);
        if (!count) return {};
    
        std::vector<Property> properties;
        properties.resize(count);
        get_details(&count, &properties.front());
        
        return properties;
    }
    
    
    template <typename Property, typename... Args>
    inline std::vector<Property> enumerate_simple_vk_property(auto fn, Args&&... args) {
        return enumerate_vk_property<Property>(
            [&](auto* count) { fn(std::forward<Args>(args)..., count, nullptr); },
            [&](auto* count, auto* props) { fn(std::forward<Args>(args)..., count, props); }
        );
    }
    
    
    template <typename Property, typename PropertyID, typename... Args>
    inline void validate_availability(const std::vector<PropertyID>& required, auto get_property_id, auto fn, Args&&... args) {
        if (required.empty()) return;
        
        for (const auto& prop : enumerate_simple_vk_property<Property>(fn, std::forward<Args>(args)...)) {
            PropertyID id = get_property_id(prop);
            
            auto selector = meta::pick<std::is_same_v<PropertyID, const char*>>(
                [&](const auto& prop) { return (bool) strcmp(prop, id); },
                [&](const auto& prop) { return prop == id; }
            );
            
            VE_ASSERT(
                contains_if(required, selector),
                "Required Vulkan property "s + to_string(id) + " is not available"
            );
        }
    }
    
    
    inline vk_resource<VkInstance> create_instance(
        SDL_Window* window,
        const std::vector<const char*>& validation_layers = {},
        const std::vector<const char*>& extensions = {}
    ) {
        // Construct application info struct.
        game_info& g_info = *game_callbacks::get_game_info();
        auto make_version = [](auto ver) { return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch); };
    
        VkApplicationInfo app_info {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = g_info.name,
            .applicationVersion = make_version(g_info.game_version),
            .pEngineName        = "VoxelEngine",
            .engineVersion      = make_version(engine::get_engine_version()),
            .apiVersion         = VK_API_VERSION_1_0
        };
    
    
        // Fetch required extensions from SDL.
        SDL_bool fetch_ext_success = SDL_TRUE;
    
        u32 extension_count = 0;
        fetch_ext_success &= SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
    
        std::vector<const char*> all_extensions;
        all_extensions.resize(extension_count);
        
        if (extension_count) {
            fetch_ext_success &= SDL_Vulkan_GetInstanceExtensions(window, &extension_count, &all_extensions.front());
        }
        
        all_extensions.insert(
            all_extensions.end(),
            extensions.begin(),
            extensions.end()
        );
        
        VE_ASSERT(fetch_ext_success, "Failed to fetch Vulkan extension list: "s + SDL_GetError());
    
        
        // Validate the presence of the required extensions.
        validate_availability<VkExtensionProperties, const char*>(
            all_extensions,
            [](const auto& prop) { return prop.extensionName; },
            vkEnumerateInstanceExtensionProperties,
            nullptr
        );
    
        
        // Validate presence of required validation layers.
        validate_availability<VkLayerProperties, const char*>(
            validation_layers,
            [](const auto& prop) { return prop.layerName; },
            vkEnumerateInstanceLayerProperties
        );
        
    
        // Create instance creation info struct.
        VkInstanceCreateInfo creation_info {
            .sType                  = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo       = &app_info,
            .enabledLayerCount      = validation_layers.size(),
            .ppEnabledLayerNames    = validation_layers.empty() ? nullptr : &validation_layers.front(),
            .enabledExtensionCount  = all_extensions.size(),
            .pEnabledExtensionNames = all_extensions.empty() ? nullptr : &all_extensions.front(),
        };
    
    
        // Construct instance.
        VkInstance instance;
        if(vkCreateInstance(&creation_info, nullptr, &instance) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan instance.");
        }
        
        
        return {
            instance,
            [](auto&& instance) { vkDestroyInstance(instance, nullptr); }
        };
    }
    
    
    inline vk_resource<VkDebugUtilsMessengerEXT> create_debug_messenger(VkInstance instance, auto callback = vulkan_error_callback) {
        VkDebugUtilsMessengerCreateInfoEXT debug_callback_info {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = vulkan_error_callback,
            .pUserData       = nullptr
        };
        
        
        auto create_callback_fn  = VE_IMPL_LOAD_VKPTR(instance, vkCreateDebugUtilsMessengerEXT);
        auto destroy_callback_fn = VE_IMPL_LOAD_VKPTR(instance, vkDestroyDebugUtilsMessengerEXT);
        
        VkDebugUtilsMessengerEXT result;
        create_callback_fn(instance, &debug_callback_info, nullptr, &result);
        
        return {
            result,
            [destroy_callback_fn, instance](auto&& messenger) { destroy_callback_fn(instance, messenger, nullptr); }
        }
    }
    
    
    inline vk_resource<VkPhysicalDevice> pick_physical_device(VkInstance instance, VkPhysicalDeviceFeatures required_features) {
        VkPhysicalDevice device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        
        auto devices = enumerate_simple_vk_property<VkPhysicalDevice>(
            vkEnumeratePhysicalDevices,
            instance
        );
        
        for (auto possible_device : devices) {
            VkPhysicalDeviceProperties possible_props;
            vkGetPhysicalDeviceProperties(possible_device, &possible_props);
            
            VkPhysicalDeviceFeatures possible_features;
            vkGetPhysicalDeviceFeatures(possible_device, &possible_features);
            
            
            auto assign_device = [&]() {
                device   = possible_device;
                props    = possible_props;
                features = possible_features;
            };
            
            
            // The device must support all the required features.
            bool missing_features = false;

            iterate_class_members<VkPhysicalDeviceFeatures>(
                [&] <std::size_t I, typename T> () {
                    bool is_required  = boost::pfr::get<I>(required_features);
                    bool is_supported = boost::pfr::get<I>(possible_features);
                    
                    if (is_required && !is_supported) missing_features = true;
                }
            );
            
            if (missing_features) continue;
            
            
            // If we don't have a device yet, pick the current one.
            if (device == VK_NULL_HANDLE) {
                assign_device();
                continue;
            }
            
            
            // Prefer discrete GPUs over integrated ones.
            if (
                props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
                possible_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            ) {
                assign_device();
                continue;
            }
            
            
            // Assume the device with more VRAM will have better performance.
            auto get_memory_size = [](auto device) {
                VkPhysicalDeviceMemoryProperties memprops;
                vkGetPhysicalDeviceMemoryProperties(device, &memprops);
                
                auto heaps = std::vector<VkMemoryHeap> {
                    memprops.memoryHeaps,
                    memprops.memoryHeaps + memprops.memoryHeapCount
                };
                
                auto it = std::find_if(
                    heaps.begin(), heaps.end(),
                    [](const auto& heap) { return heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT; }
                );
                
                return (it == heaps.end()) ? 0ull : (u64) it->size;
            };
            
            if (get_memory_size(possible_device) > get_memory_size(device)) {
                assign_device();
                continue;
            }
        }
        
        
        VE_ASSERT(
            device != VK_NULL_HANDLE,
            "Failed to find a suitable physical device with Vulkan support."
        );
        
        
        return { device, [](auto){} };
    }
    
    
    inline u32 get_queue_family(VkPhysicalDevice device, auto pred) {
        auto queue_families = enumerate_simple_vk_property<VkQueueFamilyProperties>(
            vkGetPhysicalDeviceQueueFamilyProperties,
            device
        );
        
        for (const auto& [i, family] : queue_families | views::enumerate) {
            family.
            if (pred(i, family)) return i;
        }
        
        VE_ASSERT(false, "Failed to find Vulkan queue family with the required flags: "s + std::to_string(flags));
    }
    
    
    inline u32 get_graphics_queue_family(VkPhysicalDevice device) {
        return get_queue_family(device, [](auto i, auto family) { return family & VK_QUEUE_GRAPHICS_BIT; });
    }
    
    
    inline u32 get_presentable_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface) {
        return get_queue_family(device, [&](auto i, auto family) {
            VkBool32 supported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
            return (bool) supported;
        });
    }
    
    
    inline vk_resource<VkDevice> create_logical_device(
        VkPhysicalDevice device,
        VkPhysicalDeviceFeatures required_features,
        const std::vector<u32>& queue_families,
        const std::vector<const char*>& extensions = {}
    ) {
        VE_ASSERT(!queue_families.empty(), "At least one queue family is required to create a logical device.");
        
        
        // It is possible that the queue family list contains duplicates,
        // e.g. when the graphics and presentable queue families are the same family.
        std::vector<u32> unique_families = queue_families;
        std::sort(unique_families.begin(), unique_families.end());
        unique_families.erase(std::unique(unique_families.begin(), unique_families.end()), unique_families.end());
    
    
        std::vector<VkDeviceQueueCreateInfo> queue_create_info;
        
        for (const auto& family : unique_families) {
            const float priority = 1.0f;
            
            queue_create_info.push_back(VkDeviceQueueCreateInfo {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount       = 1,
                .pQueuePriorities = &priority,
            });
        }
        
        
        VkDeviceCreateInfo device_create_info {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos       = &queue_create_info.front(),
            .queueCreateInfoCount    = queue_create_info.size(),
            .pEnabledFeatures        = &required_features,
            .enabledExtensionCount   = extensions.size(),
            .ppEnabledExtensionNames = extensions.empty() ? nullptr : &extensions.front()
        };
        
        
        VkDevice logical_device;
        VE_ASSERT(
            vkCreateDevice(device, &device_create_info, nullptr, &logical_device) !== VK_SUCCESS),
            "Failed to create Vulkan logical device."
        );


        return {
            logical_device,
            [](auto&& device) { vkDestroyDevice(device, nullptr); }
        };
    }
    
    
    inline vk_resource<VkQueue> get_queue(VkDevice device, u32 family) {
        VkQueue queue = VK_NULL_HANDLE;
        
        vkGetDeviceQueue(device, family, 0, &queue);
        VE_ASSERT(queue != VK_NULL_HANDLE, "Failed to fetch Vulkan queue for family "s + std::to_string(family));
        
        return { queue, [](auto){} };
    }
    
    
    inline vk_resource<VkSurfaceKHR> create_surface(SDL_Window* window, VkInstance instance) {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
            VE_ASSERT(false, "Failed to create Vulkan surface: "s + SDL_GetError());
        }
        
        return {
            surface,
            [instance](auto&& sf) { vkDestroySurfaceKHR(instance, sf, nullptr); }
        };
    }
    
    
    inline bool is_present_mode_available(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR mode) {
        auto modes = enumerate_simple_vk_property<VkPresentModeKHR>(
            vkGetPhysicalDeviceSurfacePresentModesKHR,
            device,
            surface
        );
        
        return contains(modes, mode);
    }
    
    
    inline VkPresentModeKHR pick_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<VkPresentModeKHR>& preference) {
        for (const auto& mode : preference) {
            if is_present_mode_available(device, surface, mode) return mode;
        }
        
        VE_LOG_WARN("None of the prefered present modes are available. Falling back to Vsync.");
        return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available.
    }
    
    
    struct swapchain_data {
        struct image_data {
            VkImage image;
            VkImageAspectFlagBits flags;
        };
        
        
        vk_resource<VkSwapchainKHR> swapchain;
        std::vector<image_data> images;
        VkExtent2D extent;
        VkFormat pixel_format;
        VkColorSpaceKHR color_space;
        
        operator VkSwapchainKHR(void) { return swapchain; }
    };
    
    
    inline std::vector<VkImage> get_swapchain_images(VkPhysicalDevice device, VkSwapchainKHR swapchain) {
        auto images = enumerate_simple_vk_property<VkImage>(
            vkGetSwapchainImagesKHR,
            device,
            swapchain
        );
    }
    
    
    inline swapchain_data create_swapchain(
        VkPhysicalDevice device,
        SDL_Window* window,
        VkSurfaceKHR surface,
        VkPresentModeKHR present_mode,
        const std::vector<VkImageAspectFlagBits>& attachments = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT }
    ) {
        VE_ASSERT(is_present_mode_available(present_mode), "Cannot create swap chain with unsupported present mode.");
        
        // Pick Color Format
        auto formats = enumerate_simple_vk_property<VkSurfaceFormatKHR>(
            vkGetPhysicalDeviceSurfaceFormatsKHR,
            device,
            surface
        );
    
        VE_ASSERT(!formats.empty(), "Cannot create Vulkan swap chain because no supported surface format was detected.");
        
        
        bool has_preferred_format = contains_if(
            formats,
            conjunction(
                ve_field_equals(format, VK_FORMAT_B8G8R8A8_SRGB),
                ve_field_equals(colorSpace, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            )
        );
        
        auto pixel_format = has_preferred_format
            ? VK_FORMAT_B8G8R8A8_SRGB:
            : formats.front().format;
        
        auto color_space = has_preferred_format
            ? VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            : formats.front().colorSpace;
        
        
        // Construct Swap Chain
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
        
        u32 num_images = std::min(
            capabilities.minImageCount + 1,
            capabilities.maxImageCount == 0 ? max_value<u32> : capabilities.maxImageCount
        );
        
        // If the graphics and presentable queues are actually the same queue,
        // the image does not have to be shared between them.
        u32 queue_families[2] = {
            get_graphics_queue_family(device),
            get_presentable_queue_family(device, surface)
        };
        
        bool require_sharing = (queue_families[0] != queue_families[1]);
    
    
        // TODO: Acquire ownership if queues are different
        // TODO: Allow swap chain recreation
        VkSwapchainCreateInfoKHR swapchain_info {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface               = surface,
            .minImageCount         = num_images,
            .imageFormat           = pixel_format,
            .imageColorSpace       = color_space,
            .imageExtent           = capabilities.currentExtent,
            .imageArrayLayers      = 1,
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode      = require_sharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = require_sharing ? 2 : 0,
            .pQueueFamilyIndices   = require_sharing ? queue_families : nullptr,
            .preTransform          = capabilities.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = present_mode,
            .clipped               = VK_TRUE,
            .oldSwapchain          = VK_NULL_HANDLE
        };
        
        
        VkSwapchainKHR swapchain;
        if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan swap chain.");
        }
        
        
        return swapchain_data {
            .swapchain = {
                swapchain,
                [device](auto&& sc) { vkDestroySwapchainKHR(device, sc, nullptr); }
            }
            .images = get_swapchain_images(device, swapchain)
                | views::zip(attachments)
                | views::transform(construct<typename swapchain_data::image_data>)
                | ranges::to<std::vector>,
            .extent       = capabilities.currentExtent,
            .pixel_format = pixel_format,
            .color_space  = color_space
        };
    }
    
    
    inline vk_resource<VkImageView> create_image_view(VkDevice device, const swapchain_data& swapchain, u32 index) {
        VkImageViewCreateInfo view_info {
            .sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image        = swapchain.images[index].image,
            .format       = swapchain.pixel_format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask     = swapchain.images[index].flags,
            .subresourceRange.baseMipLevel   = 0,
            .subresourceRange.levelCount     = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount     = 1
        };
        
        
        VkImageView view;
        if (vkCreateImageView(device, &view_info, nullptr, &view) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan image view.");
        }
        
        
        return {
            view,
            [device](auto&& view) { vkDestroyImageView(device, view, nullptr); }
        };
    }
}