#include <VoxelEngine/platform/graphics/vulkan/common.hpp>
#include <VoxelEngine/platform/graphics/vulkan/vulkan_helpers.hpp>

namespace ve::graphics {
    namespace detail {
        namespace vk_helpers = ve::detail::vk_helpers;
    
        
        // Singleton RAII object that manages setting up Vulkan when it is created and manages cleanup when it is destroyed.
        // The instance for this class is constructed as a Meyers Singleton in bind_vulkan_context.
        struct vulkan_context_raii_wrapper {
            vulkan_info info;
            
            
            vulkan_context_raii_wrapper(SDL_Window* window) {
                SDL_Vulkan_LoadLibrary(nullptr);
                
                info.instance = vk_helpers::create_instance(
                    window,
                    vulkan_settings::validation_layers,
                    vulkan_settings::global_extensions
                );
                
                if constexpr (vulkan_settings::enable_debugger) {
                    info.debug_messenger = vk_helpers::create_debug_messenger(info.instance);
                }
                
                info.physical_device = vk_helpers::pick_physical_device(
                    info.instance,
                    vulkan_settings::required_device_features
                );
            }
            
            
            ~vulkan_context_raii_wrapper(void) {
                SDL_Vulkan_UnloadLibrary();
            }
        };
    }
    
    
    vulkan_context get_or_init_vulkan_context(SDL_Window* window) {
        static detail::vulkan_context_raii_wrapper context_owner = [&](){
            VE_ASSERT(window, "Vulkan context must be initialized with a window before it can be used.");
            return detail::vulkan_context_raii_wrapper { window };
        }();
        
        return &context_owner.info;
    }
    
    
    vulkan_context bind_vulkan_context(SDL_Window* window) {
        static SDL_Window* current_owner = nullptr;
        
        auto context = get_or_init_vulkan_context(window);
        if (current_owner != window) {
            // TODO: Make window draw target.
            current_owner = window;
        }
    
        return context;
    }
    
    
    vulkan_context get_vulkan_context(void) {
        return get_or_init_vulkan_context(nullptr);
    }
}