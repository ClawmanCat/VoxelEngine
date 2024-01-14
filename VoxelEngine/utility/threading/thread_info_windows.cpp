// While wstring_convert is deprecated, the STL provides no alternative to it.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING


#include <VoxelEngine/core/core.hpp>


#if defined(VE_WINDOWS)
    #include <VoxelEngine/utility/threading/thread_info.hpp>

    #include <magic_enum.hpp>

    #include <windows.h>
    #include <processthreadsapi.h>
    #include <comdef.h>

    #include <codecvt>
    #include <locale>


    namespace ve {
        bool is_success_result(HRESULT result) {
            return SUCCEEDED(result);
        }


        std::string error_from_hresult(HRESULT result) {
            _com_error error { result };
            return std::string { error.ErrorMessage() };
        }


        std::string last_error_string(void) {
            DWORD error_code     = GetLastError();
            LPSTR message_buffer = nullptr;

            std::size_t size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error_code,
                MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                (LPTSTR) &message_buffer,
                0,
                nullptr
            );

            std::string result { message_buffer, message_buffer + size };
            LocalFree(message_buffer);

            return result;
        }


        std::expected<void, std::string> set_thread_name(std::thread::native_handle_type id, std::string_view name) {
            auto converter = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> {};
            auto wname     = converter.from_bytes(name.data(), name.data() + name.size());

            HRESULT result = SetThreadDescription(id, wname.c_str());

            return is_success_result(result)
                ? std::expected<void, std::string> {}
                : std::unexpected { error_from_hresult(result) };
        }


        std::expected<void, std::string> set_thread_priority(std::thread::native_handle_type id, thread_priority priority) {
            constexpr auto priority_converter = [] {
                std::array<int, magic_enum::enum_count<thread_priority>()> result {};

                // Note: while THREAD_MODE_BACKGROUND_BEGIN exists, a thread can only apply it to itself, so using that might cause unexpected effects.
                result[(u32) thread_priority::BACKGROUND]    = THREAD_PRIORITY_LOWEST;
                result[(u32) thread_priority::LOWEST]        = THREAD_PRIORITY_LOWEST;
                result[(u32) thread_priority::LOW]           = THREAD_PRIORITY_BELOW_NORMAL;
                result[(u32) thread_priority::NORMAL]        = THREAD_PRIORITY_NORMAL;
                result[(u32) thread_priority::HIGH]          = THREAD_PRIORITY_ABOVE_NORMAL;
                result[(u32) thread_priority::HIGHEST]       = THREAD_PRIORITY_HIGHEST;
                result[(u32) thread_priority::TIME_CRITICAL] = THREAD_PRIORITY_TIME_CRITICAL;

                return result;
            } ();


            bool success = SetThreadPriority(id, priority_converter[(u32) priority]);

            return success
                ? std::expected<void, std::string> {}
                : std::unexpected { last_error_string() };
        }
    }
#endif