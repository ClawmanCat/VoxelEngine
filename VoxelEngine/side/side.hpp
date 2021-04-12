#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    enum class side : u8 {
        CLIENT = (1 << 0),
        SERVER = (1 << 1),
        BOTH   = CLIENT | SERVER
    };
    
    VE_BITWISE_ENUM(side);
    
    
    constexpr inline side opposing(side s) {
        constexpr std::array opposites { side::SERVER, side::CLIENT };
        return opposites[((u8) s) >> 1];
    }
}