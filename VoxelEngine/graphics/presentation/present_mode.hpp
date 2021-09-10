#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx {
    enum class present_mode {
        // Single-buffer swapchain. Frames are written as they become ready. May cause tearing.
        IMMEDIATE,
        // Double-buffer FIFO swapchain (VSync). If there are no images to write to, rendering stalls.
        DOUBLE_BUFFERED,
        // Triple-buffer mailbox swapchain. If there are no images to write to, a non-presented image is overwritten.
        TRIPLE_BUFFERED
    };
}