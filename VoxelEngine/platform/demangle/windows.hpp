#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/library_loader/library_loader.hpp>

#include <typeinfo>


namespace ve::demangle {
    extern void* get_function(const char* mangled);
    extern void* get_function(const char* mangled, library_handle handle);
    
    
    template <typename Fn>
    inline const char* get_mangled_name(Fn fn) {
        return typeid(fn).raw_name();
    }
}