#pragma once

#include <VoxelEngine/core/core.hpp>


#define ve_impl_invalidate_macro(R, D, E) std::invoke(D, E);


// Can be used for classes which contain some matrix, which needs to be rebuilt when one of the class members changes.
// The method generates a getter and setter for the member with the given name,
// and the given matrices (__VA_ARGS__) are invalidated by calling std::invoke(fn, matrix)
#define ve_impl_matrix_mutator(name, fn, ...)               \
void set_##name(auto&& value) {                             \
    this->name = value;                                     \
                                                            \
    BOOST_PP_SEQ_FOR_EACH(                                  \
        ve_impl_invalidate_macro,                           \
        fn,                                                 \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
    );                                                      \
}                                                           \
                                                            \
auto get_##name(void) const {                               \
    return this->name;                                      \
}


// Same as above, but instead of a getter/setter, a method is generated to perform the given operation.
// This macro can be used to e.g. generate a move function which adds to a position rather than overwriting it.
#define ve_impl_matrix_mutator_tf(name, fn, field, op, ...) \
void name(auto&& value) {                                   \
    this->field op value;                                   \
                                                            \
    BOOST_PP_SEQ_FOR_EACH(                                  \
        ve_impl_invalidate_macro,                           \
        fn,                                                 \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
    );                                                      \
}