#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    using library_handle = void*;
    
    #ifdef VE_LINUX
        constexpr const char* library_extension = ".so";
    #else
        constexpr const char* library_extension = ".dylib";
    #endif
    
    
    namespace detail {
        extern expected<void*> get_library_function_impl(library_handle handle, const char* method);
        extern std::string get_library_error(void);
    }
    
    
    extern expected<library_handle> load_library(const char* path);
    extern void unload_library(library_handle handle);
    
    template <typename Ret, typename... Args>
    expected<Fn<Ret, Args...>> get_library_function(library_handle handle, const char* method) {
        return detail::get_library_function_impl(handle, method)
            .map([](auto ptr) { return (Fn<Ret, Args...>) ptr; });
    }
}