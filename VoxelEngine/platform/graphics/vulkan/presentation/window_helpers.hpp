#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/presentation/present_mode.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/utility/utility.hpp>

#include <magic_enum.hpp>
#include <vulkan/vulkan.hpp>
#include <SDL_video.h>
#include <SDL_vulkan.h>


namespace ve::gfx::vulkan {
    constexpr inline auto get_windowflags(void) {
        return SDL_WINDOW_VULKAN;
    }


    inline vec2ui get_canvas_size(SDL_Window* window) {
        vec2i result;
        SDL_Vulkan_GetDrawableSize(window, &result.x, &result.y);
        return (vec2ui) result;
    }


    inline present_mode present_mode_vk_to_ve(VkPresentModeKHR mode) {
        switch (mode) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR: return present_mode::IMMEDIATE;
            case VK_PRESENT_MODE_FIFO_KHR:      return present_mode::DOUBLE_BUFFERED;
            case VK_PRESENT_MODE_MAILBOX_KHR:   return present_mode::TRIPLE_BUFFERED;
            default:                            VE_ASSERT(false, "Unsupported presentation mode.");
        }

        VE_UNREACHABLE;
    }


    inline VkPresentModeKHR present_mode_ve_to_vk(present_mode mode) {
        switch (mode) {
            case present_mode::IMMEDIATE:       return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case present_mode::DOUBLE_BUFFERED: return VK_PRESENT_MODE_FIFO_KHR;
            case present_mode::TRIPLE_BUFFERED: return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }


    inline std::vector<present_mode> get_available_present_modes(VkSurfaceKHR surface) {
        auto modes = detail::enumerate_simple_vk_property<VkPresentModeKHR>(
            vkGetPhysicalDeviceSurfacePresentModesKHR,
            get_context()->physical_device,
            surface
        );

        return modes | views::transform(present_mode_vk_to_ve) | ranges::to<std::vector>;
    }


    inline present_mode pick_present_mode(VkSurfaceKHR surface, const std::vector<present_mode>& preference) {
        auto available_modes = get_available_present_modes(surface);

        for (auto mode : preference) {
            if (ranges::contains(available_modes, mode)) return mode;
        }

        // Guaranteed to be supported.
        // (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPresentModeKHR.html)
        return present_mode::DOUBLE_BUFFERED;
    }
}