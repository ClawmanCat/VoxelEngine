#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    enum class present_mode {
        // Rendering immediately continues onto the next frame after the previous frame finishes. May cause tearing.
        // Double-buffering will still be used to reduce said tearing if possible.
        IMMEDIATE,
        // Double-buffer FIFO swapchain (VSync). If there are no images to write to, rendering stalls until one becomes available.
        VSYNC,
        // Triple-buffer mailbox swapchain. If there are no images to write to, a non-presented image is overwritten.
        TRIPLE_BUFFERED
    };
}