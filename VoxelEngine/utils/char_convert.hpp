#pragma once

#include <VoxelEngine/core/core.hpp>

#include <cstddef>
#include <cstdlib>


namespace ve {
    inline std::string wchars_to_string(const wchar_t* chars) {
        std::size_t buffer_size = wcslen(chars) + 1;
        unique<char[]> buffer { new char[buffer_size] };
        
        wcstombs(buffer.get(), chars, buffer_size);
        buffer.get()[buffer_size - 1] = '\0';
        
        return std::string { buffer.get(), buffer.get() + buffer_size };
    }
}