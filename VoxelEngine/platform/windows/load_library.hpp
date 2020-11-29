#pragma once

#include <VoxelEngine/core/core.hpp>

#include <string_view>


namespace ve {
    namespace detail {
        // Avoid exposing the Windows API in a header and leaking its horrible macros everywhere.
        extern void* load_library_impl(const char* name);
        extern void  unload_library_impl(void* handle);
        extern void* get_library_fn_impl(void* handle, const char* name);
    }
    
    
    using library_handle = void*;
    constexpr inline std::string_view library_extension = ".dll";
    
    
    inline library_handle load_library(const char* name) noexcept {
        return detail::load_library_impl(name);
    }
    
    inline void unload_library(library_handle library) noexcept {
        detail::unload_library_impl(library);
    }
    
    template <typename Ret, typename... Args>
    inline Fn<Ret, Args...> get_library_fn(library_handle library, const char* name) noexcept {
        return (Fn<Ret, Args...>) detail::get_library_fn_impl(library, name);
    }
}