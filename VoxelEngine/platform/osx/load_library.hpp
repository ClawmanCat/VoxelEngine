#pragma once

#include <VoxelEngine/core/core.hpp>

#include <dlfcn.h>
#include <string_view>


namespace ve {
    using library_handle = void*;
    constexpr inline std::string_view library_extension = ".bundle";
    
    
    inline library_handle load_library(const char* name) noexcept {
        return dlopen(name, RTLD_NOW);
    }
    
    
    inline void unload_library(library_handle library) noexcept {
        dlclose(handle);
    }
    
    
    template <typename Ret, typename... Args>
    inline Fn<Ret, Args...> get_library_fn(library_handle library, const char* name) noexcept {
        return dlsym(library, name);
    }
}