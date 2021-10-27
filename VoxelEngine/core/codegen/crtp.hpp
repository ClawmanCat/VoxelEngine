#pragma once

#include <type_traits>


// Check whether or not the derived class implements the given function.
// Static version should be used for contexts where decltype(this) cannot be used.
#define VE_CRTP_IS_IMPLEMENTED(derived, fn)                                             \
(&derived::fn != &std::remove_cvref_t<decltype(*this)>::fn)

#define VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn)                                      \
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
