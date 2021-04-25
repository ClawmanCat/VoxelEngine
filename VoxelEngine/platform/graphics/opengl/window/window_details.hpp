#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/common.hpp>
#include <VoxelEngine/graphics/window/window.hpp>

#include <SDL.h>
#include <gl/glew.h>
#include <magic_enum.hpp>


namespace ve::detail::window_details {
    using namespace ve::graphics;
    
    
    struct platform_window_data {};
    
    
    struct platform_window_methods {
        static SDL_WindowFlags get_flags(void) {
            return SDL_WINDOW_OPENGL;
        }
    
        
        static void pre_window_create(window& window) {
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
        }
    
        
        static void post_window_create(window& window, window::vsync_mode vsync_mode) {
            set_vsync_mode(window, vsync_mode);
        }
        
        
        static void pre_window_destroy(window& window) {}
        
        
        static void post_window_destroy(window& window) {}
    
    
        static void pre_draw(window& window) {
            bind_opengl_context(window.get_handle());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
            const auto size = window.get_canvas_size();
            glViewport(0, 0, size.x, size.y);
        }
    
    
        static void post_draw(window& window) {
            SDL_GL_SwapWindow(window.get_handle());
        }
    
    
        static bool is_supported_vsync_mode(window& window, window::vsync_mode mode) {
            switch (mode) {
                case window::vsync_mode::IMMEDIATE:       return true;
                case window::vsync_mode::VSYNC:           return true;
                case window::vsync_mode::ADAPTIVE_VSYNC:  return glewIsSupported("GL_EXT_swap_control_tear");
                case window::vsync_mode::TRIPLE_BUFFERED: return false; // Vulkan only.
            }
        }
    
    
        static void set_vsync_mode(window& window, window::vsync_mode mode) {
            VE_ASSERT(is_supported_vsync_mode(window, mode), "Unsupported vsync mode: "s + magic_enum::enum_name(mode));
            bind_opengl_context(window.get_handle());
        
            switch (mode) {
                case window::vsync_mode::IMMEDIATE:
                    SDL_GL_SetSwapInterval(0);
                
                    break;
                case window::vsync_mode::VSYNC:
                    SDL_GL_SetSwapInterval(1);
                
                    break;
                case window::vsync_mode::ADAPTIVE_VSYNC:
                    SDL_GL_SetSwapInterval(-1);
                
                    break;
                case window::vsync_mode::TRIPLE_BUFFERED:
                    VE_ASSERT(false, "Unsupported vsync mode.");
                    break;
            }
        }
    };
}