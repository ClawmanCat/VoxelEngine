#pragma once

#include <compare>


#define VE_IMPL_COMP_FIELD_MACRO(R, D, E)    if (auto cmp = (this->E <=> D.E); cmp != 0) return cmp;
#define VE_IMPL_COMP_CATEGORY_MACRO(R, D, E) decltype(this->E <=> D.E),


#define VE_IMPL_COMPARE_AS(KwConstexpr, KwNoexcept, ...)                                                    \
[[nodiscard]] KwConstexpr auto operator<=>(const auto& other) const KwNoexcept {                            \
    using common_comparison_cat = std::common_comparison_category_t<                                        \
        BOOST_PP_SEQ_FOR_EACH(VE_IMPL_COMP_CATEGORY_MACRO, other, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))    \
        std::strong_ordering                                                                                \
    >;                                                                                                      \
                                                                                                            \
    BOOST_PP_SEQ_FOR_EACH(VE_IMPL_COMP_FIELD_MACRO, other, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__));          \
    return common_comparison_cat::equivalent;                                                               \
}                                                                                                           \
                                                                                                            \
[[nodiscard]] bool operator==(const auto& other) const { return (*this <=> other) == 0; }


#define VE_COMPARE_AS(...)          VE_IMPL_COMPARE_AS(constexpr, noexcept, __VA_ARGS__)
#define VE_RT_COMPARE_AS(...)       VE_IMPL_COMPARE_AS(/* not constexpr */, noexcept, __VA_ARGS__)
#define VE_THROWING_COMPARE_AS(...) VE_IMPL_COMPARE_AS(/* not constexpr */, /* not noexcept */, __VA_ARGS__)