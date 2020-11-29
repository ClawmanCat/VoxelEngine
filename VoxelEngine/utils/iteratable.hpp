#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>


// Pairs of (name, is_const)
#define VE_IMPL_ITERATOR_MEMBERS                                    \
((begin, 0))((end, 0))((rbegin, 0))((rend, 0))                      \
((cbegin, 1))((cend, 1))((crbegin, 1))((crend, 1))                  \
((size, 1))

#define VE_IMPL_ITERATOR_TYPEDEFS                                   \
(size_type)(difference_type)(reference)(const_reference)(iterator)  \
(const_iterator)(reverse_iterator)(const_reverse_iterator)(pointer) \
(const_pointer)(value_type)(allocator_type)


#define VE_IMPL_GEN_ITERATOR_MEMBER(Rep, Data, Elem)                \
[[nodiscard]] auto BOOST_PP_TUPLE_ELEM(0, Elem)(void)               \
BOOST_PP_EXPR_IF(BOOST_PP_TUPLE_ELEM(1, Elem), const) {             \
    return Data.BOOST_PP_TUPLE_ELEM(0, Elem)();                     \
}

#define VE_IMPL_GEN_ITERATOR_TYPEDEF(Rep, Data, Elem)               \
using Elem = typename decltype(Data)::Elem;


#define VE_MAKE_ITERATABLE(member)                                  \
BOOST_PP_SEQ_FOR_EACH(                                              \
    VE_IMPL_GEN_ITERATOR_MEMBER,                                    \
    member,                                                         \
    VE_IMPL_ITERATOR_MEMBERS                                        \
)                                                                   \
                                                                    \
BOOST_PP_SEQ_FOR_EACH(                                              \
    VE_IMPL_GEN_ITERATOR_TYPEDEF,                                   \
    member,                                                         \
    VE_IMPL_ITERATOR_TYPEDEFS                                       \
)