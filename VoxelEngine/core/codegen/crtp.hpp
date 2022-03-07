#pragma once

#include <type_traits>


namespace ve::detail {
    template <typename This> using this_type = std::remove_reference_t<std::remove_pointer_t<This>>;


    template <typename T, typename As> using const_as_t = std::conditional_t<
        std::is_const_v<As>,
        std::add_const_t<T>,
        std::remove_const_t<T>
    >;


    template <typename Derived, typename This>
    using derived_this_t = std::add_pointer_t<const_as_t<Derived, this_type<This>>>;
}


// Check whether or not the derived class implements the given function.
// Static version should be used for contexts where decltype(this) cannot be used.
#define VE_CRTP_IS_IMPLEMENTED(derived, fn)                                                     \
[] {                                                                                            \
    /* If implemented the pointer to the derived function will have a different type */         \
    /* so we won't be able to compare them directly. */                                         \
    if constexpr (std::is_same_v<                                                               \
        decltype(&derived::fn),                                                                 \
        decltype(&std::remove_cvref_t<decltype(*this)>::fn)                                     \
    >) {                                                                                        \
        return (&derived::fn != &std::remove_cvref_t<decltype(*this)>::fn);                     \
    } else {                                                                                    \
        return true;                                                                            \
    }                                                                                           \
} ()

#define VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn)                                              \
(&derived::fn != &fn)

// Equivalent to above for templated functions, since they require the 'template' keyword.
#define VE_STATIC_CRTP_IS_IMPLEMENTED_TMPL(derived, fn)                                         \
(&derived::template fn != &fn)


// Same as above, but triggers a static assert if the check fails.
#define VE_CRTP_CHECK(derived, fn)                                                              \
static_assert(                                                                                  \
    VE_CRTP_IS_IMPLEMENTED(derived, fn),                                                        \
    "Derived class does not implement required CRTP method " #fn " from base class."            \
);

#define VE_STATIC_CRTP_CHECK(derived, fn)                                                       \
static_assert(                                                                                  \
    VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn),                                                 \
    "Derived class does not implement required CRTP method " #fn " from base class."            \
);


// Invokes the derived CRTP method.
#define VE_CRTP_CALL(derived, fn, ...)                                                          \
static_cast<ve::detail::derived_this_t<derived, decltype(this)>>(this)->fn(__VA_ARGS__);

#define VE_STATIC_CRTP_CALL(derived, fn, ...)                                                   \
Derived::fn(__VA_ARGS__);


// Equivalent to above, but invocation is only attempted if the function exists.
#define VE_MAYBE_CRTP_CALL(derived, fn, ...)                                                    \
if constexpr (VE_CRTP_IS_IMPLEMENTED(derived, fn)) {                                            \
    VE_CRTP_CALL(derived, fn, __VA_ARGS__);                                                     \
}

#define VE_MAYBE_STATIC_CRTP_CALL(derived, fn, ...)                                             \
if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(derived, fn)) {                                     \
    VE_STATIC_CRTP_CALL(derived, fn, __VA_ARGS__);                                              \
}