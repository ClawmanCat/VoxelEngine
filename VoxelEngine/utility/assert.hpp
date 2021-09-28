#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/logger.hpp>

#include <SDL.h>


#if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
    #define VE_IMPL_FUNC_MACRO __PRETTY_FUNCTION__
#elif defined(VE_COMPILER_MSVC)
    #define VE_IMPL_FUNC_MACRO __FUNCSIG__
#else
    #define VE_IMPL_FUNC_MACRO __func__
#endif


#define VE_IMPL_VARIADIC_TAIL(...) \
BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TAIL(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))


namespace ve {
    // TODO: Use std::source_location once Clang supports it.
    #define VE_ASSERT(...)                                                                                          \
    [&, ve_impl_where = VE_IMPL_FUNC_MACRO, ve_impl_line = __LINE__, ve_impl_file = __FILE__] () {                  \
        if (!(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__))) [[unlikely]] {                                               \
            auto header    = "A VoxelEngine assertion failed. The engine will now terminate.";                      \
            auto location  = ve::cat("At ", ve_impl_where, " in ", ve_impl_file, " at line ", ve_impl_line);        \
            auto condition = ve::cat_with(" ", VE_IMPL_VARIADIC_TAIL(__VA_ARGS__));                                 \
            auto message   = ve::cat_with("\n", header, location, condition);                                       \
                                                                                                                    \
            VE_LOG_FATAL(message);                                                                                  \
                                                                                                                    \
            SDL_ShowSimpleMessageBox(                                                                               \
                SDL_MESSAGEBOX_ERROR,                                                                               \
                "Assertion failure.",                                                                               \
                message.c_str(),                                                                                    \
                nullptr                                                                                             \
            );                                                                                                      \
                                                                                                                    \
            VE_BREAKPOINT;                                                                                          \
            std::exit(-1);                                                                                          \
        }                                                                                                           \
    }();
    
    
    #ifdef VE_DEBUG
        #define VE_DEBUG_ASSERT(...) VE_ASSERT(__VA_ARGS__)
    #else
        #define VE_DEBUG_ASSERT(...)
    #endif
}