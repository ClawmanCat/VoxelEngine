#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    constexpr bool constexpr_strcmp(const char* a, const char* b) {
        std::size_t i = 0;
        
        while (a[i] != '\0' || b[i] != '\0') {
            if (a[i] != b[i]) return false;
            ++i;
        }
        
        return true;
    }
    
    
    constexpr std::size_t constexpr_strlen(const char* str) {
        std::size_t i = 0;
        while (str[i] != '\0') ++i;
        return i;
    }
}