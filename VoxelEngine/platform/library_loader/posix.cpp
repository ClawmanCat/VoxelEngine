#include <VoxelEngine/platform/library_loader/library_loader.hpp>
#include <VoxelEngine/utility/string.hpp>


#if defined(VE_LINUX) || defined(VE_APPLE)
    #include <dlfcn.h>


    namespace ve {
        native_library_handle load_library(const fs::path& path) {
            auto result = dlopen(path.c_str(), RTLD_GLOBAL | RTLD_LAZY | RTLD_DEEPBIND);
            if (!result) throw std::runtime_error(cat("Failed to load library (", path.filename(), "): ", get_last_error()));
            
            return result;
        }
        
        
        void unload_library(native_library_handle handle) {
            dlclose(handle);
        }
        
        
        void* load_library_symbol(native_library_handle handle, const char* symbol) {
            auto result = dlsym(handle, symbol);
            if (!result) throw std::runtime_error(cat("Failed to load symbol ", symbol, " from library: ", get_last_error()));
            
            return result;
        }
        
        
        bool is_library_file(const fs::path& path) {
            #ifdef VE_LINUX
                auto file_extension = ".so";
            #else
                auto file_extension = ".dylib";
            #endif
            
            return path.extension() == file_extension;
        }
        
        
        std::string get_last_error(void) {
            const char* error = dlerror();
            
            if (error) return std::string { error };
            else return "No further information available.";
        }
    }
#endif