#pragma once

#include <VoxelEngine/core/preprocessor.hpp>
#include <VoxelEngine/core/typedef/pointer.hpp>


#pragma region helper_macros

#define ve_impl_param_list_macro(rep, data, tuple)                      \
BOOST_PP_TUPLE_ELEM(0, tuple) BOOST_PP_TUPLE_ELEM(1, tuple)             \
BOOST_PP_EXPR_IF(                                                       \
    BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple), 3),                      \
    = BOOST_PP_TUPLE_ELEM(2, tuple)                                     \
)                                                                       \
BOOST_PP_COMMA_IF(BOOST_PP_LESS(rep, data))

#define ve_impl_param_list(...)                                         \
BOOST_PP_SEQ_FOR_EACH(                                                  \
    ve_impl_param_list_macro,                                           \
    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),                                \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                               \
)


#define ve_impl_param_elem_macro(rep, data, tuple) (BOOST_PP_TUPLE_ELEM(data, tuple))

#define ve_impl_param_elem(index, ...)                                  \
BOOST_PP_SEQ_FOR_EACH(                                                  \
    ve_impl_param_elem_macro,                                           \
    index,                                                              \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                               \
)


#define ve_impl_forward_seq_macro(z, n, data) (fwd(BOOST_PP_SEQ_ELEM(n, data)))

// Can't use BOOST_PP_SEQ_FOR_EACH since it'd be a recursive call.
#define ve_impl_forward_seq(seq)                                        \
BOOST_PP_REPEAT(                                                        \
    BOOST_PP_SEQ_SIZE(seq),                                             \
    ve_impl_forward_seq_macro,                                          \
    seq                                                                 \
)

#pragma endregion helper_macros




// Forces the given class to be only constructable as a shared pointer.
// This prevents undefined behaviour for classes that inherit from std::enable_shared_from_this.
// Note that the hidden type is required, as the constructor must be public for make_shared to work.
// Usage: ve_shared_only(my_class, args...) : some_member(arg) { ... }
//
// Since ve_shared_only is commonly used in conjunction with std::enable_shared_from_this,
// it is sometimes desirable to construct the pointer first, so shared_from_this() is valid,
// and then initialize the object.
// For this purpose the _then version of this macro can be used to call some method after the constructor.
// Usage:
//    ve_shared_only_then(my_class, init, args...) : some_member(arg) { ... }
//    void init(void) { ... }
#define ve_impl_shared_only(cls, then, ...)                             \
private:                                                                \
    struct hidden_constructor_t {                                       \
        explicit hidden_constructor_t(void) = default;                  \
    };                                                                  \
                                                                        \
public:                                                                 \
    static ve::shared<cls> create(auto&&... args) {                     \
        auto ptr = ve::make_shared<cls>(                                \
            hidden_constructor_t{},                                     \
            fwd(args)...                                                \
        );                                                              \
                                                                        \
        then;                                                           \
        return ptr;                                                     \
    }                                                                   \
                                                                        \
    cls(hidden_constructor_t __VA_OPT__(,) __VA_ARGS__)


#define ve_shared_only(cls, ...) ve_impl_shared_only(cls, /* no op */, __VA_ARGS__)
#define ve_shared_only_then(cls, then, ...) ve_impl_shared_only(cls, ptr->then(), __VA_ARGS__)


// Equivalent to above, but allows default parameters and multiple constructors.
// Parameters should be passed as (type, name) or (type, name, value)
#define ve_impl_complex_shared_only(cls, then, ...)                     \
private:                                                                \
    struct BOOST_PP_CAT(hidden_constructor_t_, __LINE__) {              \
        explicit BOOST_PP_CAT(hidden_constructor_t_, __LINE__)(void)    \
            = default;                                                  \
    };                                                                  \
                                                                        \
public:                                                                 \
    static ve::shared<cls> create(ve_impl_param_list(__VA_ARGS__)) {    \
        auto ptr = ve::make_shared<cls>(                                \
            BOOST_PP_CAT(hidden_constructor_t_, __LINE__){},            \
            /* fwd(name) for every tuple in __VA_ARGS__ */              \
            BOOST_PP_SEQ_ENUM(                                          \
                ve_impl_forward_seq(                                    \
                    ve_impl_param_elem(1, __VA_ARGS__)                  \
                )                                                       \
            )                                                           \
        );                                                              \
                                                                        \
        then;                                                           \
        return ptr;                                                     \
    }                                                                   \
                                                                        \
    cls(                                                                \
        BOOST_PP_CAT(hidden_constructor_t_, __LINE__)                   \
        __VA_OPT__(,) ve_impl_param_list(__VA_ARGS__)                   \
    )


#define ve_complex_shared_only(cls, ...) ve_impl_complex_shared_only(cls, /* no op */, __VA_ARGS__)
#define ve_complex_shared_only_then(cls, then, ...) ve_impl_complex_shared_only(cls, ptr->then(), __VA_ARGS__)