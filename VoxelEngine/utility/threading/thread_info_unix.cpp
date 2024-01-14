#include <VoxelEngine/core/core.hpp>


#if defined (VE_UNIX_LIKE)
    #include <VoxelEngine/utility/threading/thread_info.hpp>

    #include <magic_enum.hpp>

    #include <pthread.h>
    #include <sched.h>


    namespace ve {
        extern std::expected<void, std::string> set_thread_name(std::thread::native_handle_type id, std::string_view name) {
            std::string buffer { name };
            int result = 0;


            #if defined(VE_APPLE)
                std::thread::native_handle_type current_thread;
                pthread_threadid_np(nullptr, &current_thread);

                if (id != current_thread) {
                    return std::unexpected { "On OSX a thread may only change its own name." };
                } else {
                    result = pthhread_set_name_np(buffer.c_str());
                }
            #elif defined(VE_LINUX)
                result = pthread_set_name_np(id, buffer.c_str());
            #else
                return std::unexpected { "Unsupported platform." };
            #endif


            if      (result == 0)      return {};
            else if (result == ERANGE) return std::unexpected { "Name too long." };
            else                       return std::unexpected { "Unknown error." };
        }


        extern std::expected<void, std::string> set_thread_priority(std::thread::native_handle_type id, thread_priority priority) {
            return std::unexpected { "Not yet implemented." };
        }
    }
#endif