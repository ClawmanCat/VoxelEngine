#include <VoxelEngine/platform/library_loader/windows.hpp>


#if defined(VE_WINDOWS)
    #include <Windows.h>
    #include <libloaderapi.h>
    
    
    namespace ve {
        expected<library_handle> load_library(const char* path) {
            auto handle = LoadLibraryA(path);
            
            if (handle) return handle;
            else return make_unexpected(detail::get_library_error());
        }
        
        
        void unload_library(library_handle handle) {
            FreeLibrary((HMODULE) handle);
        }
        
        
        expected<void*> detail::get_library_function_impl(library_handle handle, const char* method) {
            auto ptr = GetProcAddress((HMODULE) handle, method);
            
            if (ptr) return ptr;
            else return make_unexpected(detail::get_library_error());
        }
        
        
        std::string detail::get_library_error(void) {
            LPVOID message_ptr;
            DWORD error_code = GetLastError();
            
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM     |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error_code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &message_ptr,
                0,
                nullptr
            );
            
            std::string result {
                (const char*) message_ptr,
                (const char*) message_ptr + lstrlen((LPCTSTR) message_ptr)
            };
            
            LocalFree(message_ptr);
            return result;
        }
    }
#endif