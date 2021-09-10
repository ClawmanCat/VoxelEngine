#include <VoxelEngine/platform/graphics/vulkan/presentation/canvas.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve::gfx::vulkan {
    canvas::canvas(window* window) : owner(window) {
        if (!SDL_Vulkan_CreateSurface(window->get_handle(), get_context()->instance, &handle)) {
            VE_ASSERT(false, "Failed to create Vulkan surface.");
        }

        queues = create_window_queues(
            get_context()->instance,
            get_context()->physical_device,
            get_context()->logical_device,
            handle
        );
    }


    canvas::~canvas(void) {
        if (handle) vkDestroySurfaceKHR(get_context()->instance, handle, nullptr);
    }
}