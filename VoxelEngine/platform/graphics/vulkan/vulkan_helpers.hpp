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


// Loads the function pointer for the given Vulkan function dynamically.
#define VE_IMPL_LOAD_VKPTR(instance, name)                                      \
[&]() {                                                                         \
    auto ptr = (PFN_##name) vkGetInstanceProcAddr(instance, #name);             \
    VE_ASSERT(ptr, "Failed to load " #name " method.");                         \
    return ptr;                                                                 \
}()


namespace ve::detail::vk_helpers {
    using namespace ve::graphics;
    
    
    struct swapchain_data {
        struct image_data {
            VkImage image;
            VkImageUsageFlagBits flags;
        };
        
        
        vk_resource<VkSwapchainKHR> swapchain;
        std::vector<image_data> images;
        VkExtent2D extent;
        VkFormat pixel_format;
        VkColorSpaceKHR color_space;
        
        operator VkSwapchainKHR(void) { return swapchain; }
    };
    
    
    template <typename T> inline std::string to_string(const T& v) {
        std::stringstream s;
        s << v;
        return s.str();
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
    
    
    inline u32 get_queue_family(VkPhysicalDevice device, auto pred) {
        auto queue_families = enumerate_simple_vk_property<VkQueueFamilyProperties>(
            vkGetPhysicalDeviceQueueFamilyProperties,
            device
        );
        
        for (const auto& [i, family] : queue_families | views::enumerate) {
            if (pred(i, family)) return i;
        }
        
        VE_ASSERT(false, "Failed to find Vulkan queue family with the required settings.");
        VE_UNREACHABLE;
    }
    
    
    extern VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_error_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        u32 type, // VkDebugUtilsMessageTypeFlagBitsEXT
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* user_data
    );
    
    
    extern vk_resource<VkInstance> create_instance(
        SDL_Window* window,
        const std::vector<const char*>& validation_layers = {},
        const std::vector<const char*>& extensions = {}
    );
    
    
    extern vk_resource<VkDebugUtilsMessengerEXT> create_debug_messenger(
        VkInstance instance,
        decltype(vulkan_error_callback) callback = vulkan_error_callback
    );
    
    
    extern vk_resource<VkPhysicalDevice> pick_physical_device(
        VkInstance instance, 
        VkPhysicalDeviceFeatures required_features
    );
    
    
    extern u32 get_graphics_queue_family(
        VkPhysicalDevice device
    );
    
    
    extern u32 get_presentable_queue_family(
        VkPhysicalDevice device, 
        VkSurfaceKHR surface
    );
    
    
    extern vk_resource<VkDevice> create_logical_device(
        VkPhysicalDevice device,
        VkPhysicalDeviceFeatures required_features,
        const std::vector<u32>& queue_families,
        const std::vector<const char*>& extensions = {}
    );
    
    
    extern vk_resource<VkQueue> get_queue(
        VkDevice device, 
        u32 family
    );
    
    
    extern vk_resource<VkSurfaceKHR> create_surface(
        SDL_Window* window, 
        VkInstance instance
    );
    
    
    extern bool is_present_mode_available(
        VkPhysicalDevice device, 
        VkSurfaceKHR surface, 
        VkPresentModeKHR mode
    );
    
    
    extern VkPresentModeKHR pick_present_mode(
        VkPhysicalDevice device, 
        VkSurfaceKHR surface, 
        const std::vector<VkPresentModeKHR>& preference
    );
    
    
    extern std::vector<VkImage> get_swapchain_images(
        VkDevice device,
        VkSwapchainKHR swapchain
    );
    
    
    extern swapchain_data create_swapchain(
        VkPhysicalDevice device,
        SDL_Window* window,
        VkSurfaceKHR surface,
        VkPresentModeKHR present_mode,
        const std::vector<VkImageUsageFlagBits>& attachments
    );
    
    
    extern vk_resource<VkImageView> create_image_view(
        VkDevice device,
        const swapchain_data& swapchain,
        u32 index
    );
}