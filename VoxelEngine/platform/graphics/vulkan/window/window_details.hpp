#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/window/window.hpp>
#include <VoxelEngine/platform/graphics/vulkan/common.hpp>
#include <VoxelEngine/platform/graphics/vulkan/vulkan_helpers.hpp>

#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>


namespace ve::detail::window_details {
    using namespace ve::graphics;
    
    
    struct platform_window_data {
        // These are RAII objects. Do not reorder them, as it would change the destructor call order.
        vk_resource<VkSurfaceKHR> surface;
        vk_resource<VkDevice> logical_device;
        vk_resource<VkQueue> graphics_queue, presentable_queue;
        vk_resource<VkSwapchainKHR> swapchain;
        vk_resource<VkFramebuffer> framebuffer;
        vk_resource<VkPipeline> pipeline;
    };
    
    
    struct platform_window_methods {
        static VkPresentModeKHR translate_vsync_mode(window::vsync_mode mode) {
            switch (mode) {
                case window::vsync_mode::IMMEDIATE:       return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case window::vsync_mode::VSYNC:           return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case window::vsync_mode::ADAPTIVE_VSYNC:  return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case window::vsync_mode::TRIPLE_BUFFERED: return VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
        
        
        static SDL_WindowFlags get_flags(void) {
            return SDL_WINDOW_VULKAN;
        }
        
        
        static void pre_window_create(window& window) {}
    
        
        static void post_window_create(window& window, window::vsync_mode vsync_mode) {
            auto* ctx = bind_vulkan_context(window);
            
            window.platform_data = new platform_window_data();
            auto& windata = *window.platform_data;
            
            windata.surface = vk_helpers::create_surface(window.get_handle(), ctx->instance);
            
            windata.logical_device = vk_helpers::create_logical_device(
                ctx->physical_device,
                vulkan_settings::required_device_features,
                {
                    vk_helpers::get_graphics_queue_family(ctx->physical_device),
                    vk_helpers::get_presentable_queue_family(ctx->physical_device, windata.surface)
                },
                vulkan_settings::device_extensions
            );
            
            windata.graphics_queue = vk_helpers::get_queue(
                windata.logical_device,
                vk_helpers::get_graphics_queue_family(ctx->physical_device)
            );
            
            windata.presentable_queue = vk_helpers::get_queue(
                windata.logical_device,
                vk_helpers::get_presentable_queue_family(ctx->physical_device, windata.surface)
            );
            
            windata.swapchain = vk_helpers::create_swapchain(
                ctx->physical_device,
                window.get_handle(),
                windata.surface,
                translate_vsync_mode(vsync_mode)
            );
        }
    
        
        static void pre_window_destroy(window& window) { }
    
        
        static void post_window_destroy(window& window) {
            delete window.platform_data;
        }
        
        
        static void pre_draw(window& window) {
            // TODO
        }
        
        
        static void post_draw(window& window) {
            // TODO
        }
    
    
        static bool is_supported_vsync_mode(window& window, window::vsync_mode mode) {
            auto* ctx = get_vulkan_context();
            
            auto supported = [&](auto mode) {
                return vk_helpers::is_present_mode_available(
                    ctx->physical_device,
                    window.platform_data->surface,
                    mode
                );
            };
            
            return supported(translate_vsync_mode(mode));
        }
        
        
        static void set_vsync_mode(window& window, window::vsync_mode mode) {
            // TODO
        }
    };
}