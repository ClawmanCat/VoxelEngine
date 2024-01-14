#pragma once

#include <VoxelEngine/core/preprocessor.hpp>

#include <boost/preprocessor.hpp>




// Copy / Move fields
#define VE_IMPL_COPY_FIELD_MACRO(R, D, E) this->E = D.E;
#define VE_IMPL_MOVE_FIELD_MACRO(R, D, E) this->E = std::move(D.E);

#define VE_COPY_FIELDS(Source, ...) BOOST_PP_SEQ_FOR_EACH(VE_IMPL_COPY_FIELD_MACRO, Source, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define VE_MOVE_FIELDS(Source, ...) BOOST_PP_SEQ_FOR_EACH(VE_IMPL_MOVE_FIELD_MACRO, Source, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))




// Templates for getter and setter methods.
#define VE_IMPL_GETTER(KwConstexpr, KwNoexcept, KwStatic, KwConst, ValueCategory, Name)                 \
[[nodiscard]] KwConstexpr KwStatic ValueCategory get_##Name(void) KwConst KwNoexcept {                  \
    return Name;                                                                                        \
}

#define VE_IMPL_SETTER(KwConstexpr, KwNoexcept, KwStatic, KwConst, ValueCategory, Name)                 \
KwConstexpr KwStatic void set_##Name(auto&& value) KwNoexcept {                                         \
    Name = fwd(value);                                                                                  \
}




// Forwards the required arguments to the correct macro.
#define VE_IMPL_GET_SET_MACRO(R, D, E)                                                                  \
BOOST_PP_TUPLE_ELEM(0, D)(                                                                              \
    BOOST_PP_TUPLE_ELEM(1, D),                                                                          \
    BOOST_PP_TUPLE_ELEM(2, D),                                                                          \
    BOOST_PP_TUPLE_ELEM(3, D),                                                                          \
    BOOST_PP_TUPLE_ELEM(4, D),                                                                          \
    BOOST_PP_TUPLE_ELEM(5, D),                                                                          \
    E                                                                                                   \
)

// Base macro to generate getters or setters for the given names with the given keyword arguments.
#define VE_IMPL_GEN_GETTERS_SETTERS(GS, KwConstexpr, KwNoexcept, KwStatic, KwConst, ValueCategory, ...) \
BOOST_PP_SEQ_FOR_EACH(                                                                                  \
    VE_IMPL_GET_SET_MACRO,                                                                              \
    (GS, KwConstexpr, KwNoexcept, KwStatic, KwConst, ValueCategory),                                    \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                               \
)




// Getters
#define VE_GET_VALS(...)         VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, /* not static */, const,           auto,        __VA_ARGS__)
#define VE_GET_CREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, /* not static */, const,           const auto&, __VA_ARGS__)
#define VE_GET_MREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, /* not static */, /* not const */, auto&,       __VA_ARGS__) VE_GET_CREFS(__VA_ARGS__)
#define VE_GET_STATIC_VALS(...)  VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, static,           /* not const */, auto,        __VA_ARGS__)
#define VE_GET_STATIC_CREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, static,           /* not const */, const auto&, __VA_ARGS__)
#define VE_GET_STATIC_MREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, constexpr, noexcept, static,           /* not const */, auto&,       __VA_ARGS__)

#define VE_GET_RT_VALS(...)         VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, /* not static */, const,           auto,        __VA_ARGS__)
#define VE_GET_RT_CREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, /* not static */, const,           const auto&, __VA_ARGS__)
#define VE_GET_RT_MREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, /* not static */, /* not const */, auto&,       __VA_ARGS__) VE_GET_RT_CREFS(__VA_ARGS__)
#define VE_GET_RT_STATIC_VALS(...)  VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, static,           /* not const */, auto,        __VA_ARGS__)
#define VE_GET_RT_STATIC_CREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, static,           /* not const */, const auto&, __VA_ARGS__)
#define VE_GET_RT_STATIC_MREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, noexcept, static,           /* not const */, auto&,       __VA_ARGS__)

#define VE_GET_THROWING_VALS(...)         VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, /* not static */, const,           auto,        __VA_ARGS__)
#define VE_GET_THROWING_CREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, /* not static */, const,           const auto&, __VA_ARGS__)
#define VE_GET_THROWING_MREFS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, /* not static */, /* not const */, auto&,       __VA_ARGS__) VE_GET_THROWING_CREFS(__VA_ARGS__)
#define VE_GET_THROWING_STATIC_VALS(...)  VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, static,           /* not const */, auto,        __VA_ARGS__)
#define VE_GET_THROWING_STATIC_CREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, static,           /* not const */, const auto&, __VA_ARGS__)
#define VE_GET_THROWING_STATIC_MREFS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_GETTER, /* not constexpr */, /* not noexcept */, static,           /* not const */, auto&,       __VA_ARGS__)


// Setters
#define VE_SET_VALS(...)                 VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, constexpr,           noexcept,          /* not static */, /* ignored */, /* ignored */, __VA_ARGS__)
#define VE_SET_STATIC_VALS(...)          VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, constexpr,           noexcept,          static,           /* ignored */, /* ignored */, __VA_ARGS__)
#define VE_SET_RT_VALS(...)              VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, /* not constexpr */, noexcept,          /* not static */, /* ignored */, /* ignored */, __VA_ARGS__)
#define VE_SET_RT_STATIC_VALS(...)       VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, /* not constexpr */, noexcept,          static,           /* ignored */, /* ignored */, __VA_ARGS__)
#define VE_SET_THROWING_VALS(...)        VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, /* not constexpr */, /* not noexcept*/, /* not static */, /* ignored */, /* ignored */, __VA_ARGS__)
#define VE_SET_THROWING_STATIC_VALS(...) VE_IMPL_GEN_GETTERS_SETTERS(VE_IMPL_SETTER, /* not constexpr */, /* not noexcept*/, static,           /* ignored */, /* ignored */, __VA_ARGS__)


// Combined Getters + Setters
#define VE_GET_SET_VALS(...)                  VE_GET_VALS(__VA_ARGS__) VE_SET_VALS(__VA_ARGS__)
#define VE_GET_SET_CREFS(...)                 VE_GET_CREFS(__VA_ARGS__) VE_SET_VALS(__VA_ARGS__)
#define VE_GET_SET_MREFS(...)                 VE_GET_MREFS(__VA_ARGS__) VE_SET_VALS(__VA_ARGS__)
#define VE_GET_SET_STATIC_VALS(...)           VE_GET_STATIC_VALS(__VA_ARGS__) VE_SET_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_STATIC_CREFS(...)          VE_GET_STATIC_CREFS(__VA_ARGS__) VE_SET_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_STATIC_MREFS(...)          VE_GET_STATIC_MREFS(__VA_ARGS__) VE_SET_STATIC_VALS(__VA_ARGS__)

#define VE_GET_SET_RT_VALS(...)               VE_GET_RT_VALS(__VA_ARGS__) VE_SET_RT_VALS(__VA_ARGS__)
#define VE_GET_SET_RT_CREFS(...)              VE_GET_RT_CREFS(__VA_ARGS__) VE_SET_RT_VALS(__VA_ARGS__)
#define VE_GET_SET_RT_MREFS(...)              VE_GET_RT_MREFS(__VA_ARGS__) VE_SET_RT_VALS(__VA_ARGS__)
#define VE_GET_SET_RT_STATIC_VALS(...)        VE_GET_RT_STATIC_VALS(__VA_ARGS__) VE_SET_RT_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_RT_STATIC_CREFS(...)       VE_GET_RT_STATIC_CREFS(__VA_ARGS__) VE_SET_RT_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_RT_STATIC_MREFS(...)       VE_GET_RT_STATIC_MREFS(__VA_ARGS__) VE_SET_RT_STATIC_VALS(__VA_ARGS__)

#define VE_GET_SET_THROWING_VALS(...)         VE_GET_THROWING_VALS(__VA_ARGS__) VE_SET_THROWING_VALS(__VA_ARGS__)
#define VE_GET_SET_THROWING_CREFS(...)        VE_GET_THROWING_CREFS(__VA_ARGS__) VE_SET_THROWING_VALS(__VA_ARGS__)
#define VE_GET_SET_THROWING_MREFS(...)        VE_GET_THROWING_MREFS(__VA_ARGS__) VE_SET_THROWING_VALS(__VA_ARGS__)
#define VE_GET_SET_THROWING_STATIC_VALS(...)  VE_GET_THROWING_STATIC_VALS(__VA_ARGS__) VE_SET_THROWING_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_THROWING_STATIC_CREFS(...) VE_GET_THROWING_STATIC_CREFS(__VA_ARGS__) VE_SET_THROWING_STATIC_VALS(__VA_ARGS__)
#define VE_GET_SET_THROWING_STATIC_MREFS(...) VE_GET_THROWING_STATIC_MREFS(__VA_ARGS__) VE_SET_THROWING_STATIC_VALS(__VA_ARGS__)