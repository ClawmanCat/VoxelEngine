#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/vector.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan::detail {
    #define VE_IMPL_VK_SWITCH_CASE(out, entry) \
    case entry: out = #entry; break;

    #define VE_IMPL_VK_SWITCH_CASE_SET_LVL(level_out, level_val, out, entry) \
    case entry: out = #entry; level_out = logger::level::level_val; break;


    inline VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        u32 type, // VkDebugUtilsMessageTypeFlagBitsEXT
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
            default: VE_UNREACHABLE;
        }

        severity_str.remove_prefix("VK_DEBUG_UTILS_MESSAGE_SEVERITY_"sv.length());
        severity_str.remove_suffix("_BIT_EXT"sv.length());


        loggers::get_gfxapi_logger().message(
            cat("[", type_str, " ", severity_str, "] ", data->pMessage),
            level
        );


        #ifdef VE_DEBUG
            if (level == logger::level::ERROR) VE_BREAKPOINT;
        #endif


        return VK_FALSE;
    }


    inline VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_setup_info(decltype(debug_messenger_callback) callback) {
        return VkDebugUtilsMessengerCreateInfoEXT {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = callback,
            .pUserData       = nullptr
        };
    }


    inline vk_resource<VkDebugUtilsMessengerEXT> create_debug_messenger(
        VkInstance instance,
        decltype(debug_messenger_callback) callback
    ) {
        auto debug_callback_info = get_debug_messenger_setup_info(callback);


        VkDebugUtilsMessengerEXT result;
        auto create_callback_fn = VE_VKPTR(instance, vkCreateDebugUtilsMessengerEXT);
        create_callback_fn(instance, &debug_callback_info, nullptr, &result);


        return {
            std::move(result),
            bind<0, 2>(
                VE_VKPTR(instance, vkDestroyDebugUtilsMessengerEXT),
                instance, nullptr
            )
        };
    }
}