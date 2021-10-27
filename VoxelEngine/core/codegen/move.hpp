#pragma once

#include <boost/preprocessor.hpp>

#include <utility>


// Make the class move only with defaulted move operators.
#define ve_impl_move_only(cls, kw_constexpr)                \
cls(const cls&) = delete;                                   \
cls& operator=(const cls&) = delete;                        \
                                                            \
kw_constexpr cls(cls&&) noexcept = default;                 \
kw_constexpr cls& operator=(cls&&) noexcept = default;

#define ve_move_only(cls) ve_impl_move_only(cls, constexpr)
#define ve_rt_move_only(cls) ve_impl_move_only(cls, )



// Move the object by swapping the contents of the other object.
#define ve_impl_swap_move(R, D, E) std::swap(E, o.E);

#define ve_impl_swap_move_only(cls, kw_constexpr, ...)      \
cls(const cls&) = delete;                                   \
cls& operator=(const cls&) = delete;                        \
                                                            \
kw_constexpr cls(cls&& o) noexcept {                        \
    *this = std::move(o);                                   \
}                                                           \
                                                            \
kw_constexpr cls& operator=(cls&& o) noexcept {             \
    BOOST_PP_SEQ_FOR_EACH(                                  \
        ve_impl_swap_move,                                  \
        _,                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
    )                                                       \
                                                            \
    return *this;                                           \
}

#define ve_swap_move_only(cls, ...) ve_impl_swap_move_only(cls, constexpr, __VA_ARGS__)
#define ve_rt_swap_move_only(cls, ...) ve_impl_swap_move_only(cls, , __VA_ARGS__)


// Inherit the base class move operators.
#define ve_impl_inherit_move(cls, base, kw_constexpr)       \
kw_constexpr cls(cls&& o) : base(std::move(o)) {}           \
                                                            \
kw_constexpr cls& operator=(cls&& o) {                      \
    base::operator=(std::move(o));                          \
    return *this;                                           \
}

#define ve_inherit_move(cls, base) ve_impl_inherit_move(cls, base, constexpr)
#define ve_rt_inherit_move(cls, base) ve_impl_inherit_move(cls, base, )


// Make the object non-copyable and immovable.
// Useful for objects that give out pointers to themselves.
#define ve_immovable(cls)                                   \
cls(const cls&) = delete;                                   \
cls(cls&&) = delete;                                        \
                                                            \
cls& operator=(const cls&) = delete;                        \
cls& operator=(cls&&) = delete;