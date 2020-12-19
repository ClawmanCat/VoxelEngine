#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>

#include <type_traits>


namespace ve::detail {
    template <typename> struct crtp_traits {};
    
    template <template <typename> typename B, typename D> struct crtp_traits<B<D>> {
        template <typename Derived> using base = B<Derived>;
        using Derived = D;
        using Self = B<D>;
    };
}


// Prevents CRTP derived classes from calling the base method if they don't implement the method themselves.
#define VE_CRTP_GUARD(fn)                                                                       \
static_assert(                                                                                  \
    (                                                                                           \
        &ve::detail::crtp_traits<std::remove_cvref_t<decltype(*this)>>::Self::fn !=             \
        &ve::detail::crtp_traits<std::remove_cvref_t<decltype(*this)>>::Derived::fn             \
    ),                                                                                          \
    "CRTP method " #fn " not implemented by deriving class."                                    \
)


// Calls the given CRTP overloaded method.
// Usage: VE_CRTP_WRAP(SomeFn)(Args...);
// Note: this produces a return statement!
#define VE_CRTP_WRAP(fn)                                                                        \
VE_CRTP_GUARD(fn);                                                                              \
                                                                                                \
using BOOST_PP_CAT(ve_impl_qualified_derived_t_, __LINE__) = std::remove_reference_t<           \
    typename ve::detail::crtp_traits<std::remove_cvref_t<decltype(*this)>>::Derived             \
>;                                                                                              \
                                                                                                \
return static_cast<BOOST_PP_CAT(ve_impl_qualified_derived_t_, __LINE__)*>(this)->fn