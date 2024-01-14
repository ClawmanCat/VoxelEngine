#pragma once

#include <VoxelEngine/core/preprocessor.hpp>

#include <boost/preprocessor.hpp>


// Templates for generating wrapping constants, typedefs and functions.
#define VE_IMPL_WRAP_CONSTANT_MACRO(R, D, E) constexpr static inline auto E = D::E;
#define VE_IMPL_WRAP_TYPEDEF_MACRO(R, D, E)  using E = typename D::E;

#define VE_IMPL_WRAP_MEM_FN(KwConstexpr, KwStatic, KwNoexcept, KwConst, Accessor, Member, Fn) \
KwConstexpr KwStatic decltype(auto) Fn(auto&&... args) KwConst KwNoexcept { return Member Accessor Fn(fwd(args)...); }

#define VE_IMPL_WRAP_MEM_FN_MACRO(R, D, E)  \
VE_IMPL_WRAP_MEM_FN(                        \
    BOOST_PP_TUPLE_ELEM(0, D),              \
    BOOST_PP_TUPLE_ELEM(1, D),              \
    BOOST_PP_TUPLE_ELEM(2, D),              \
    BOOST_PP_TUPLE_ELEM(3, D),              \
    BOOST_PP_TUPLE_ELEM(4, D),              \
    BOOST_PP_TUPLE_ELEM(5, D),              \
    E                                       \
)


// Wrapper macros.
#define VE_WRAP_CONSTANTS(Member, ...) BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_CONSTANT_MACRO, Member, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_TYPEDEFS(Member, ...)  BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_TYPEDEF_MACRO,  Member, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define VE_WRAP_MEM_FNS(Member, ...)                BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (constexpr,           /* not static */, noexcept,           /* not const */, .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_CONST_MEM_FNS(Member, ...)          BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (constexpr,           /* not static */, noexcept,           const,           .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_RT_MEM_FNS(Member, ...)             BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, /* not static */, noexcept,           /* not const */, .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_RT_CONST_MEM_FNS(Member, ...)       BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, /* not static */, noexcept,           const,           .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_THROWING_MEM_FNS(Member, ...)       BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, /* not static */, /* not noexcept */, /* not const */, .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_THROWING_CONST_MEM_FNS(Member, ...) BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, /* not static */, /* not noexcept */, const,           .,  Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_STATIC_FNS(Member, ...)             BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (constexpr,           static,           noexcept,           /* not const */, ::, Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_RT_STATIC_FNS(Member, ...)          BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, static,           noexcept,           /* not const */, ::, Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_WRAP_THROWING_STATIC_FNS(Member, ...)    BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (/* not constexpr */, static,           /* not noexcept */, /* not const */, ::, Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define VE_WRAP_FNS_WITH_KEYWORDS(KwConstexpr, KwStatic, KwNoexcept, KwConst, Accessor, Member, ...) \
BOOST_PP_SEQ_FOR_EACH(VE_IMPL_WRAP_MEM_FN_MACRO, (KwConstexpr, KwStatic, KwNoexcept, KwConst, Accessor, Member), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))