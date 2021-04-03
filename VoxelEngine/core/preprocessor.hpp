#pragma once

#include <VoxelEngine/core/configuration.hpp>


// Marks the given code path as unreachable.
#if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
    #define VE_UNREACHABLE __builtin_unreachable()
#else
    #define VE_UNREACHABLE __assume(0)
#endif


#if defined(VE_COMPILER_CLANG) || defined(VE_COMPILER_GCC)
    #define VE_PURE __attribute__((const))
#else
    #define VE_PURE
#endif


// Forces initialization order for a static variable.
// Prefer using local statics where possible.
#define ve_init_order(Priority) __attribute__((init_priority(Priority + 101)))


// Check whether or not the derived class implements the given function.
// Static version should be used for contexts where decltype(this) cannot be used.
#define VE_CRTP_IS_IMPLEMENTED(derived, fn) \
(&derived::fn != &std::remove_cvref_t<decltype(*this)>::fn)

#define VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn) \
(&derived::fn != &fn)

// Same as above, but triggers a static assert if the check fails.
#define VE_CRTP_CHECK(derived, fn)                                                      \
static_assert(                                                                          \
    VE_CRTP_IS_IMPLEMENTED(derived, fn),                                                \
    "Derived class does not implement required CRTP method " #fn " from base class."    \
);

#define VE_STATIC_CRTP_CHECK(derived, fn)                                               \
static_assert(                                                                          \
    VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn),                                         \
    "Derived class does not implement required CRTP method " #fn " from base class."    \
);



// Expands to nothing.
#define VE_EMPTY(...)

// Can be used instead of commas in preprocessor macros.
#define VE_COMMA ,

// Can be used to unwrap typenames passed inside parentheses to prevent commas from splitting the macro argument.
namespace ve::detail {
    template <typename> struct unwrap { };
    
    template <typename X> struct unwrap<X(void)> { using type = void; };
    template <typename X, typename Y> struct unwrap<X(Y)> { using type = Y; };
}

#define VE_UNWRAP(...) \
typename ve::detail::unwrap<void(__VA_ARGS__)>::type

// When expanding a class' base classes, typename is not allowed.
#define VE_UNWRAP_BASE(...) \
ve::detail::unwrap<void(__VA_ARGS__)>::type

