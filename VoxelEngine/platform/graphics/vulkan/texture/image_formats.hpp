#pragma once

#include <VoxelEngine/core/core.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct image_format_info {
        u8 stride;
        u8 channels;
        std::array<u8, 4> channel_depths;

        VkImageAspectFlags aspects = 0;
    };


    // TODO: Support additional formats.
    const inline hash_map<VkFormat, image_format_info> image_formats {
        std::pair { VK_FORMAT_UNDEFINED,                  image_format_info { .stride = 0,  .channels = 0, .channel_depths = { 0,  0,  0,  0  } } },
        // 8-bit RGBA
        std::pair { VK_FORMAT_R8_UNORM,                   image_format_info { 8,  1, { 8,  0,  0,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_R8G8_UNORM,                 image_format_info { 16, 2, { 8,  8,  0,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_R8G8B8_UNORM,               image_format_info { 24, 3, { 8,  8,  8,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_R8G8B8A8_UNORM,             image_format_info { 32, 4, { 8,  8,  8,  8  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        // 8-bit RGB(A) with SRGB
        std::pair { VK_FORMAT_R8G8B8_SRGB,                image_format_info { 24, 3, { 8,  8,  8,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_R8G8B8A8_SRGB,              image_format_info { 32, 4, { 8,  8,  8,  8  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        // Single-channel F16 and F32
        std::pair { VK_FORMAT_R16_SFLOAT,                 image_format_info { 16, 1, { 16, 0,  0,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_R32_SFLOAT,                 image_format_info { 32, 1, { 32, 0,  0,  0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        // Common HDR formats
        std::pair { VK_FORMAT_R16G16B16_SFLOAT,           image_format_info { 48, 3, { 16, 16, 16, 0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        std::pair { VK_FORMAT_B10G11R11_UFLOAT_PACK32,    image_format_info { 32, 3, { 10, 11, 11, 0  }, VK_IMAGE_ASPECT_COLOR_BIT } },
        // Depth buffer
        std::pair { VK_FORMAT_D16_UNORM,                  image_format_info { 16, 1, { 16, 0,  0,  0  }, VK_IMAGE_ASPECT_DEPTH_BIT } },
        std::pair { VK_FORMAT_D32_SFLOAT,                 image_format_info { 32, 1, { 32, 0,  0,  0  }, VK_IMAGE_ASPECT_DEPTH_BIT } },
        std::pair { VK_FORMAT_X8_D24_UNORM_PACK32,        image_format_info { 32, 2, { 8,  24, 0,  0  }, VK_IMAGE_ASPECT_DEPTH_BIT } }
    };
}