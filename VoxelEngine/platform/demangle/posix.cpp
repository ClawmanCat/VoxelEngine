#include <VoxelEngine/platform/demangle/posix.hpp>


#if defined(VE_LINUX) || defined(VE_APPLE)
    #include <dlfcn.h>
    
    
    namespace ve::demangle {
        void* get_function(const char* mangled) {
            static void* self_handle = dlopen(nullptr, RTLD_LAZY);
            return get_function(mangled, self_handle);
        }
        
        
        void* get_function(const char* mangled, library_handle handle) {
            return dlsym(handle, mangled);
        }
    }
#endif