#include <VoxelEngine/platform/library_loader/library_loader.hpp>
#include <VoxelEngine/utility/string.hpp>


#ifdef VE_WINDOWS
    #include <Windows.h>
    
    
    namespace ve {
        native_library_handle load_library(const fs::path& path) {
            // Backslashes in the path will get escaped, so the final string will contain 4 backslashes for every directory.
            // LoadLibraryA does not like this. Fortunately, we can just use forward slashes instead.
            std::string path_string = replace_substring(path.string(), "\\\\", "/");
            
            auto result = (void*) LoadLibraryA((LPCSTR) path_string.c_str());
            if (!result) throw std::runtime_error(cat("Failed to load library (", path.filename(), "): ", get_last_error()));
            
            return result;
        }
        
        
        void unload_library(native_library_handle handle) {
            FreeLibrary((HMODULE) handle);
        }
        
        
        void* load_library_symbol(native_library_handle handle, const char* symbol) {
            auto result = (void*) GetProcAddress((HMODULE) handle, (LPCSTR) symbol);
            if (!result) throw std::runtime_error(cat("Failed to load symbol ", symbol, " from library: ", get_last_error()));
            
            return result;
        }
    
    
        bool is_library_file(const fs::path& path) {
            return path.extension() == ".dll";
        }
    
    
        std::string get_last_error(void) {
            DWORD error_code = GetLastError();
            LPTSTR error_msg = nullptr;
    
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error_code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &error_msg,
                0,
                nullptr
            );
            
            if (error_msg) {
                std::string result_msg { (const char*) error_msg };
                LocalFree(error_msg);
                
                return result_msg;
            } else {
                return "No further information available.";
            }
        }
    }
#endif