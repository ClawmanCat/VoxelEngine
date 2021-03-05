#include <VoxelEngine/platform/demangle/windows.hpp>


#if defined(VE_WINDOWS)
    #include <Windows.h>
    
    
    namespace ve::demangle {
        void* get_function(const char* mangled) {
            void* handle = (void*) GetCurrentProcess();
            return get_function(mangled, handle);
        }
    
    
        void* get_function(const char* mangled, library_handle handle) {
            return (void*) GetProcAddress((HMODULE) handle, mangled);
        }
    }
#endif