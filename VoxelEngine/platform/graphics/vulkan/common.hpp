#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/resource.hpp>
#include <VoxelEngine/utility/overloadable_setting.hpp>

#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>


namespace ve::graphics {
    // Change these settings by specializing overloadable_vulkan_settings<overloaded_settings_tag>.
    template <typename = void> struct overloadable_vulkan_settings {
        constexpr static inline VkPhysicalDeviceFeatures required_device_features {
            .fullDrawIndexUint32 = VK_TRUE,
            .geometryShader      = VK_TRUE,
            .tessellationShader  = VK_TRUE,
            .dualSrcBlend        = VK_TRUE,
            .logicOp             = VK_TRUE,
            .depthClamp          = VK_TRUE,
            .multiViewport       = VK_TRUE,
            .shaderUniformBufferArrayDynamicIndexing = VK_TRUE
        };
        
        
        const static inline std::vector<const char*> validation_layers {
            VE_DEBUG_ONLY("VK_LAYER_KHRONOS_validation")
        };
        
        const static inline std::vector<const char*> global_extensions {
            VE_DEBUG_ONLY("VK_EXT_debug_utils")
        };
    
        const static inline std::vector<const char*> device_extensions {
            "VK_KHR_swapchain"
        };
        
        
        constexpr static inline bool enable_debugger =
            VE_DEBUG_ONLY(true)
            VE_RELEASE_ONLY(false);
    };
    
    using vulkan_settings = overloadable_vulkan_settings<overloaded_settings_tag>;
    
    
    struct vulkan_info {
        // These are RAII objects. Do not reorder them, as it would change the destructor call order.
        vk_resource<VkInstance> instance;
        vk_resource<VkDebugUtilsMessengerEXT> debug_messenger;
        vk_resource<VkPhysicalDevice> physical_device;
    };
    
    
    using vulkan_context = vulkan_info*;
    extern vulkan_context bind_vulkan_context(SDL_Window* window);
    extern vulkan_context get_vulkan_context(void);
}