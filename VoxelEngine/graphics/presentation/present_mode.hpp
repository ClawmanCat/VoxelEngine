#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    struct present_mode_t {
        std::string_view name;
        u32 min_images, preferred_images;

        ve_field_comparable(present_mode_t, name);
    };


    namespace present_mode {
        // Rendering immediately continues onto the next frame after the previous frame finishes. May cause tearing.
        // Double-buffering will still be used to reduce said tearing if possible.
        constexpr static inline present_mode_t IMMEDIATE {
            .name = "immediate mode",
            .min_images = 1,
            .preferred_images = 3
        };


        // Double-buffered FIFO swapchain (VSync).
        // If there are no images to write to, rendering stalls until one becomes available.
        constexpr static inline present_mode_t VSYNC {
            .name = "VSync",
            .min_images = 2,
            .preferred_images = 3
        };


        // Triple-buffer mailbox swapchain.
        // If there are no images to write to, a non-presented image is overwritten.
        constexpr static inline present_mode_t MAILBOX {
            .name = "triple buffered",
            .min_images = 3,
            .preferred_images = 3
        };
    };
}