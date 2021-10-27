#pragma once

#include <compare>

#include <boost/preprocessor.hpp>


namespace ve::detail {
    template <typename... Ts> using comparison_category_for_t = std::common_comparison_category_t<
        std::compare_three_way_result_t<Ts>...
    >;
}


#define ve_impl_eq_comparable(cls, kw_constexpr)            \
kw_constexpr bool operator==(const cls&) const = default;   \
kw_constexpr bool operator!=(const cls&) const = default;

#define ve_eq_comparable(cls) ve_impl_eq_comparable(cls, constexpr)
#define ve_rt_eq_comparable(cls) ve_impl_eq_comparable(cls, /* not constexpr */)


#define ve_impl_comparable(cls, kw_constexpr)               \
kw_constexpr bool operator== (const cls&) const = default;  \
kw_constexpr bool operator!= (const cls&) const = default;  \
kw_constexpr auto operator<=>(const cls&) const = default;

#define ve_comparable(cls) ve_impl_comparable(cls, constexpr)
#define ve_rt_comparable(cls) ve_impl_comparable(cls, /* not constexpr */)



#define ve_impl_compare_field(R, D, E) if (auto cmp = (E D o.E); cmp != 0) return (common_t) cmp;
#define ve_impl_typeof_field(R, D, E) ( decltype(E) )

#define ve_impl_field_comparable(cls, kw_constexpr, ...)    \
kw_constexpr bool operator==(const cls& o) const {          \
    using common_t = bool;                                  \
                                                            \
    BOOST_PP_SEQ_FOR_EACH(                                  \
        ve_impl_compare_field,                              \
        ==,                                                 \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
    );                                                      \
                                                            \
    return true;                                            \
}                                                           \
                                                            \
kw_constexpr bool operator!=(const cls& o) const {          \
    return !(*this == o);                                   \
}                                                           \
                                                            \
kw_constexpr auto operator<=>(const cls& o) const {         \
    using common_t = ve::detail::comparison_category_for_t< \
        BOOST_PP_SEQ_ENUM(                                  \
            BOOST_PP_SEQ_FOR_EACH(                          \
                ve_impl_typeof_field,                       \
                _,                                          \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
            )                                               \
        )                                                   \
    >;                                                      \
                                                            \
    BOOST_PP_SEQ_FOR_EACH(                                  \
        ve_impl_compare_field,                              \
        <=>,                                                \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
    );                                                      \
                                                            \
    return common_t::equivalent;                            \
}

#define ve_field_comparable(cls, ...) ve_impl_field_comparable(cls, constexpr, __VA_ARGS__)
#define ve_rt_field_comparable(cls, ...) ve_impl_field_comparable(cls, /* not constexpr */, __VA_ARGS__)