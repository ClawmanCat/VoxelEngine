#pragma once

#include <VoxelEngine/core/configuration.hpp>

#include <boost/preprocessor.hpp>

#include <string_view>
#include <cstdlib>
#include <type_traits>


#define VE_IMPL_VARIADIC_ALL_BUT_LAST(...) \
BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TAIL(BOOST_PP_SEQ_REVERSE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))))

#define VE_IMPL_VARIADIC_LAST(...) \
BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))


// Usage: VE_ASSERT(x == y || some_fn(x, y, z), "Assertion failed message!");
// Note: if the assertion message contains exposed commas (i.e. not contained within a string or parentheses),
// the assertion message should be surrounded by parentheses.
#ifdef VE_DEBUG
    #define VE_ASSERT(...)                                                                                      \
    [&, ve_impl_where = __PRETTY_FUNCTION__, ve_impl_line = __LINE__](){                                        \
        if (!((bool) (VE_IMPL_VARIADIC_ALL_BUT_LAST(__VA_ARGS__)))) [[unlikely]] {                              \
            VE_LOG_FATAL("A VoxelEngine assertion failed. The engine will now terminate.");                     \
            VE_LOG_FATAL("Triggered at "s + ve_impl_where + " on line " + std::to_string(ve_impl_line) + ":");  \
            VE_LOG_FATAL([&]() -> std::string { return VE_IMPL_VARIADIC_LAST(__VA_ARGS__); }(), false);         \
                                                                                                                \
            std::terminate();                                                                                   \
        }                                                                                                       \
    }()
#else
    #define VE_ASSERT(...)
#endif