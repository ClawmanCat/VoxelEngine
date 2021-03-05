#include <VoxelEngine/platform/library_loader/posix.hpp>


#if defined(VE_LINUX) || defined(VE_APPLE)
    #include <dlfcn.h>
    
    
    namespace ve {
        expected<library_handle> load_library(const char* path) {
            auto handle = dlopen(path, RTLD_NOW);
            
            if (handle) return handle;
            else return make_unexpected(detail::get_library_error());
        }
        
        
        void unload_library(library_handle handle) {
            dlclose(handle);
        }
        
        
        expected<void*> detail::get_library_function_impl(library_handle handle, const char* method) {
            auto ptr = dlsym(handle, name);
            
            if (ptr) return ptr;
            else return make_unexpected(detail::get_library_error());
        }
        
        
        std::string detail::get_library_error(void) {
            return std::string { dlerror() };
        }
    }
#endif