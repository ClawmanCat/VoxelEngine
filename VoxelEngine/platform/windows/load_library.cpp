#include <VoxelEngine/platform/windows/load_library.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


namespace ve::detail {
    void* load_library_impl(const char* name) {
        return LoadLibraryA(name);
    }
    
    
    void unload_library_impl(void* handle) {
        FreeLibrary((HMODULE) handle);
    }
    
    
    void* get_library_fn_impl(void* handle, const char* name) {
        return (void*) GetProcAddress((HMODULE) handle, name);
    }
}