#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <type_traits>


// Boost Asio destroys iterators to strings that have already been deleted.
// With MSVC 8, there exists a known issue that causes crashes when this happens in debug mode, which Boost Asio handles.
// The very same issue seems to be occurring with the most recent version of MSVC however, so pretend we're on MSVC 8 if we're on Windows.
// (This problem also affects Clang on Windows, since it uses the MSVC STL.)
// See also 'buffer_debug_check' in boost/asio/buffer.hpp.
#if defined(VE_COMPILER_MSVC) || defined(VE_COMPILER_WINCLANG)
    #define BOOST_ASIO_MSVC 1400
#endif


// Abseil uses std::result_of, which was removed in C++20.
// TODO: Remove this once Abseil is updated as this is UB!
namespace std {
    template <typename T> class result_of {};
    
    template <typename Fn, typename... Args> struct result_of<Fn(Args...)> {
        using type = std::invoke_result_t<Fn, Args...>;
    };
    
    template <typename T> using result_of_t = typename std::result_of<T>::type;
}