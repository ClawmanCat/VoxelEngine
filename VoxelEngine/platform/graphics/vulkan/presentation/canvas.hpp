#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/vulkan/object/resource.hpp>
#include <VoxelEngine/platform/graphics/vulkan/presentation/swapchain.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/queue_data.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/vector.hpp>
#include <VoxelEngine/utility/functional.hpp>

#include <vulkan/vulkan.hpp>
#include <SDL_video.h>
#include <SDL_vulkan.h>


namespace ve::gfx {
    class window;
}


namespace ve::gfx::vulkan {
    class canvas {
    public:
        canvas(void) = default;
        canvas(window* window);
        ~canvas(void);

        ve_swap_move_only(canvas, handle, queues);


        VE_GET_CREF(handle);
        VE_GET_MREF(queues);
    private:
        window* owner = nullptr;

        VkSurfaceKHR handle = VK_NULL_HANDLE;
        window_queues queues;
    };
}