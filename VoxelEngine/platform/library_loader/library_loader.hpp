#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>


namespace ve {
    using native_library_handle = void*;
    
    extern native_library_handle load_library(const fs::path& path);
    extern void unload_library(native_library_handle handle);
    extern void* load_library_symbol(native_library_handle handle, const char* symbol);
    extern bool is_library_file(const fs::path& path);
    extern std::string get_last_error(void);
    
    
    template <typename Ret, typename... Args>
    inline fn<Ret, Args...> load_library_function(native_library_handle handle, const char* name) {
        return (fn<Ret, Args...>) load_library_symbol(handle, name);
    }
    
    
    class library_handle {
    public:
        library_handle(void) = default;
        explicit library_handle(const fs::path& path) : handle(load_library(path)) {}
        
        ~library_handle(void) {
            if (handle) unload_library(handle);
        }
        
        ve_swap_move_only(library_handle, handle);
    
    
        template <typename Ret, typename... Args>
        inline fn<Ret, Args...> get_function(const char* name) {
            return (fn<Ret, Args...>) load_library_symbol(handle, name);
        }
    
        template <typename Ret, typename... Args>
        inline Ret call(const char* name, meta::dont_deduce<Args>... args) {
            return get_function<Ret, Args...>(name)(fwd(args)...);
        }
        
        operator bool(void) const { return handle; }
        
        native_library_handle native_handle(void) const { return handle; }
    private:
        native_library_handle handle = nullptr;
    };
}