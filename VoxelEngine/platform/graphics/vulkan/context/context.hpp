#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>

#include <boost/pfr.hpp>
#include <vulkan/vulkan.hpp>
#include <SDL_video.h>


namespace ve::gfx {
    class window;
}


namespace ve::gfx::vulkan {
    namespace detail {
        template <typename T> using pick_fn = std::function<const T&(const std::vector<T>&)>;


        // Return the first item from the preference list that is available
        // or the first available item if none of the preferred items are available.
        constexpr auto preference_or_first(auto&&... preference) {
            return [...preference = fwd(preference)] (const auto& options) -> decltype(auto) {
                for (const auto& pref : std::array { preference... }) {
                    auto it = ranges::find_if(options, bind_front(ve_wrap_callable(boost::pfr::eq), pref));
                    if (it != options.end()) return *it;
                }

                return options.at(0);
            };
        }
    }


    struct vulkan_settings {
        VkPhysicalDeviceFeatures required_device_features;
        std::vector<std::string> validation_layers, global_extensions, device_extensions;
        bool enable_debugger;

        detail::pick_fn<VkSurfaceFormatKHR> pick_swapchain_surface_format;
        detail::pick_fn<VkFormat> pick_swapchain_depth_format;
    };


    const inline vulkan_settings default_vulkan_settings {
        .required_device_features = VkPhysicalDeviceFeatures {
            .fullDrawIndexUint32 = VK_TRUE,
            .geometryShader      = VK_TRUE,
            .tessellationShader  = VK_TRUE,
            .dualSrcBlend        = VK_TRUE,
            .logicOp             = VK_TRUE,
            .depthClamp          = VK_TRUE,
            .fillModeNonSolid    = VK_TRUE,
            .wideLines           = VK_TRUE,
            .largePoints         = VK_TRUE,
            .multiViewport       = VK_TRUE,
            .shaderUniformBufferArrayDynamicIndexing = VK_TRUE
        },

        .validation_layers                       = { VE_DEBUG_ONLY("VK_LAYER_KHRONOS_validation") },
        .global_extensions                       = { VE_DEBUG_ONLY("VK_EXT_debug_utils") },
        .device_extensions                       = { "VK_EXT_extended_dynamic_state", "VK_EXT_index_type_uint8", "VK_KHR_swapchain" },
        .enable_debugger                         = VE_IF_DEBUG_ELSE(true, false),
        .pick_swapchain_surface_format           = detail::preference_or_first(VkSurfaceFormatKHR { VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR }),
        .pick_swapchain_depth_format             = detail::preference_or_first(VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT),
    };


    struct context {
        vulkan_settings settings;

        vk_resource<VkInstance> instance;
        vk_resource<VkPhysicalDevice> physical_device;
        vk_resource<VkDevice> logical_device;
        global_queues queues;

        // Only present if created with settings.enable_debugger = true.
        vk_resource<VkDebugUtilsMessengerEXT> debugger;

        std::list<window*> vulkan_windows;
        window* active_window;
    };


    extern bool     is_context_created(void);
    extern context* get_context(void);
    extern context* get_or_create_context(SDL_Window* initial_window, const vulkan_settings* settings = &default_vulkan_settings);
}