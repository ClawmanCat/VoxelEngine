#pragma once

#include <VoxelEngine/core/definitions/c_syntax.hpp>

#include <type_traits>
#include <functional>


namespace ve::detail {
    template <typename D, typename F> struct function_traits {};

    template <typename D, typename R, typename... A> struct function_traits<D, fn<R, A...>> {
        using signature = R(A...);
    };

    template <typename D, typename C, typename R, typename... A> struct function_traits<D, mem_fn<C, R, A...>> {
        using signature = R(D&, A...);
    };

    template <typename D, typename C, typename R, typename... A> struct function_traits<D, const_mem_fn<C, R, A...>> {
        using signature = R(const D&, A...);
    };


    template <typename This, typename T> using const_as_this = std::conditional_t<
        std::is_const_v<std::remove_pointer_t<This>>,
        const T,
        T
    >;
}




/**
 * @def VE_IMPL_DERIVED_SIGNATURE
 * Expands to the signature of the provided function, with the this-parameter (if one exists) replaced with that of the derived class.
 * Template arguments may be passed as part of the function name.
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 */
#define VE_IMPL_DERIVED_SIGNATURE(This, Derived, Fn, KwTemplate) \
typename ve::detail::function_traits<Derived, decltype(&This::KwTemplate Fn)>::signature


/**
 * @def VE_IMPL_CRTP_IS_OVERRIDDEN
 * Checks if the given CRTP base method is overridden in the derived class.
 * Optionally this method can also check if the signature matches. In this case the signature should be passed as __VA_ARGS__.
 * Template arguments may be passed as part of the function name.
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param __VA_ARGS__ Optional signature of the method to check the derived method signature against.
 */
#define VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, KwTemplate, ...)                                              \
[] {                                                                                                                \
    /* Perform optional signature check. */                                                                         \
    __VA_OPT__(                                                                                                     \
        if constexpr (!requires { std::function<__VA_ARGS__> { &Derived::KwTemplate Fn }; }) return false;          \
    )                                                                                                               \
                                                                                                                    \
    /* There are three possible cases we have to check:                                             */              \
    /* 1. Function is not overloaded -> &Derived::Fn == &This::Fn                                   */              \
    /* 2. Function is overloaded with a different signature -> Function pointers are incomparable.  */              \
    /* 3. Function is overloaded with the same signature -> &Derived::Fn != &This::Fn               */              \
    if constexpr (std::is_same_v<decltype(&This::KwTemplate Fn), decltype(&Derived::KwTemplate Fn)>) {              \
        /* Same types -> compare addresses. */                                                                      \
        return (&This::KwTemplate Fn != &Derived::KwTemplate Fn);                                                   \
    } else {                                                                                                        \
        /* Different types -> guaranteed to be overloaded. */                                                       \
        return true;                                                                                                \
    }                                                                                                               \
} ()


/**
 * @def VE_IMPL_CRTP_ASSERT
 * Asserts that the given CRTP base method is overridden by the derived class.
 * Optionally this method can also check if the signature matches. In this case the signature should be passed as __VA_ARGS__.
 * Template arguments may be passed as part of the function name.
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param EnableAssert If false the assert will always pass. Used to simplify other macros using this one.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param __VA_ARGS__ Optional signature of the method to check the derived method signature against.
 */
#define VE_IMPL_CRTP_ASSERT(This, Derived, Fn, EnableAssert, KwTemplate, ...)                                       \
static_assert(                                                                                                      \
    !EnableAssert || VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, KwTemplate, __VA_ARGS__),                        \
    "CRTP method " #Fn " from CRTP base class " #This " is not overridden in derived class " #Derived "."           \
)


/**
 * @def VE_IMPL_CRTP_CALL
 * Calls the derived CRTP member method, optionally asserting that it is implemented first.
 * Optionally this method can also check if the signature matches. In this case the signature should be passed as __VA_ARGS__.
 * Template arguments may be passed as part of the function name.
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param AssertImplemented If true, a static assert will be inserted to check if the derived class overrides the function.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param ArgTuple A tuple of arguments to pass to the derived method.
 * @param __VA_ARGS__ Optional signature of the method to check the derived method signature against.
 */
#define VE_IMPL_CRTP_CALL(This, Derived, Fn, AssertImplemented, KwTemplate, ArgTuple, ...)                          \
[&] {                                                                                                               \
    VE_IMPL_CRTP_ASSERT(This, Derived, Fn, AssertImplemented, KwTemplate, __VA_ARGS__);                             \
    return static_cast<ve::detail::const_as_this<decltype(this), Derived> *>(this)->KwTemplate Fn ArgTuple;         \
} ()


/**
 * @def VE_IMPL_STATIC_CRTP_CALL
 * Calls the derived static CRTP method, optionally asserting that it is implemented first.
 * Optionally this method can also check if the signature matches. In this case the signature should be passed as __VA_ARGS__.
 * Template arguments may be passed as part of the function name.
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param AssertImplemented If true, a static assert will be inserted to check if the derived class overrides the function.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param ArgTuple A tuple of arguments to pass to the derived method.
 * @param __VA_ARGS__ Optional signature of the method to check the derived method signature against.
 */
#define VE_IMPL_STATIC_CRTP_CALL(This, Derived, Fn, AssertImplemented, KwTemplate, ArgTuple, ...)                   \
[&] {                                                                                                               \
    VE_IMPL_CRTP_ASSERT(This, Derived, Fn, AssertImplemented, KwTemplate, __VA_ARGS__);                             \
    return Derived::KwTemplate Fn ArgTuple;                                                                         \
} ()


/**
 * @def VE_IMPL_TRY_CRTP_CALL
 * Performs a CRTP call only if the given CRTP member method is overridden, otherwise returns the optional default value (__VA_ARGS__).
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param ArgTuple A tuple of arguments to pass to the derived method.
 * @param __VA_ARGS__ Optional default value to return if the method is not overridden in the derived class.
 */
#define VE_IMPL_TRY_CRTP_CALL(This, Derived, Fn, KwTemplate, ArgTuple, ...)                                         \
[&] {                                                                                                               \
    if constexpr (VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, KwTemplate)) {                                      \
        return VE_IMPL_CRTP_CALL(This, Derived, Fn, false, KwTemplate, ArgTuple);                                   \
    } else return __VA_ARGS__;                                                                                      \
} ()


/**
 * @def VE_IMPL_TRY_STATIC_CRTP_CALL
 * Performs a CRTP call only if the given CRTP static method is overridden, otherwise returns the optional default value (__VA_ARGS__).
 * @param This The typename of the current class.
 * @param Derived The typename of the derived class.
 * @param Fn The name of the CRTP-method.
 * @param KwTemplate If the method is a function template, the keyword 'template', otherwise empty.
 * @param ArgTuple A tuple of arguments to pass to the derived method.
 * @param __VA_ARGS__ Optional default value to return if the method is not overridden in the derived class.
 */
#define VE_IMPL_TRY_STATIC_CRTP_CALL(This, Derived, Fn, KwTemplate, ArgTuple, ...)                                  \
[&] {                                                                                                               \
    if constexpr (VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, KwTemplate)) {                                      \
        return VE_IMPL_STATIC_CRTP_CALL(This, Derived, Fn, false, KwTemplate, ArgTuple);                            \
    } else return __VA_ARGS__;                                                                                      \
} ()


// Public facing overloads for VE_IMPL_CRTP_ASSERT.
#define VE_CRTP_IS_OVERRIDDEN(This, Derived, Fn)                                    VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, /* no template */, VE_IMPL_DERIVED_SIGNATURE(This, Derived, Fn, /* no template*/))
#define VE_CRTP_IS_TEMPLATE_OVERRIDDEN(This, Derived, Fn)                           VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, template,          VE_IMPL_DERIVED_SIGNATURE(This, Derived, Fn, /* no template*/))
#define VE_CRTP_IS_OVERRIDDEN_WITH_SIGNATURE(This, Derived, Fn, Signature)          VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, /* no template */, Signature)
#define VE_CRTP_IS_TEMPLATE_OVERRIDDEN_WITH_SIGNATURE(This, Derived, Fn, Signature) VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, template,          Signature)
#define VE_CRTP_IS_OVERRIDDEN_WITH_ANY_SIGNATURE(This, Derived, Fn)                 VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, /* no template */)
#define VE_CRTP_IS_TEMPLATE_OVERRIDDEN_WITH_ANY_SIGNATURE(This, Derived, Fn)        VE_IMPL_CRTP_IS_OVERRIDDEN(This, Derived, Fn, template)


// Public facing overloads for VE_IMPL_CRTP_ASSERT.
#define VE_CRTP_ASSERT_OVERRIDDEN(This, Derived, Fn)                                    VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, /* no template */, VE_IMPL_DERIVED_SIGNATURE(This, Derived, Fn, /* no template*/))
#define VE_CRTP_ASSERT_TEMPLATE_OVERRIDDEN(This, Derived, Fn)                           VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, template,          VE_IMPL_DERIVED_SIGNATURE(This, Derived, Fn, /* no template*/))
#define VE_CRTP_ASSERT_OVERRIDDEN_WITH_SIGNATURE(This, Derived, Fn, Signature)          VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, /* no template */, Signature)
#define VE_CRTP_ASSERT_TEMPLATE_OVERRIDDEN_WITH_SIGNATURE(This, Derived, Fn, Signature) VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, template,          Signature)
#define VE_CRTP_ASSERT_OVERRIDDEN_WITH_ANY_SIGNATURE(This, Derived, Fn)                 VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, /* no template */)
#define VE_CRTP_ASSERT_TEMPLATE_OVERRIDDEN_WITH_ANY_SIGNATURE(This, Derived, Fn)        VE_IMPL_CRTP_ASSERT(This, Derived, Fn, true, template)


// Public facing overloads for VE_IMPL_CRTP_CALL and VE_IMPL_STATIC_CRTP_CALL.
#define VE_CRTP_CALL(This, Derived, Fn, ...)                 VE_IMPL_CRTP_CALL(This, Derived, Fn, true, /* no template */, (__VA_ARGS__))
#define VE_CRTP_TEMPLATE_CALL(This, Derived, Fn, ...)        VE_IMPL_CRTP_CALL(This, Derived, Fn, true, template,          (__VA_ARGS__))
#define VE_CRTP_STATIC_CALL(This, Derived, Fn, ...)          VE_IMPL_STATIC_CRTP_CALL(This, Derived, Fn, true, /* no template */, (__VA_ARGS__))
#define VE_CRTP_STATIC_TEMPLATE_CALL(This, Derived, Fn, ...) VE_IMPL_STATIC_CRTP_CALL(This, Derived, Fn, true, template,          (__VA_ARGS__))


// Public facing overloads for VE_IMPL_TRY_CRTP_CALL and VE_IMPL_TRY_STATIC_CRTP_CALL.
#define VE_TRY_CRTP_CALL(This, Derived, Fn, ArgTuple, ...)                 VE_IMPL_TRY_CRTP_CALL(This, Derived, Fn, /* no template */, ArgTuple, __VA_ARGS__)
#define VE_TRY_TEMPLATE_CRTP_CALL(This, Derived, Fn, ArgTuple, ...)        VE_IMPL_TRY_CRTP_CALL(This, Derived, Fn, template,          ArgTuple, __VA_ARGS__)
#define VE_TRY_STATIC_CRTP_CALL(This, Derived, Fn, ArgTuple, ...)          VE_IMPL_TRY_STATIC_CRTP_CALL(This, Derived, Fn, /* no template */, ArgTuple, __VA_ARGS__)
#define VE_TRY_STATIC_TEMPLATE_CRTP_CALL(This, Derived, Fn, ArgTuple, ...) VE_IMPL_TRY_STATIC_CRTP_CALL(This, Derived, Fn, template,          ArgTuple, __VA_ARGS__)