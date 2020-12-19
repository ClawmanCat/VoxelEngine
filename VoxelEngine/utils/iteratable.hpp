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
[[nodiscard]] decltype(Data.BOOST_PP_TUPLE_ELEM(0, Elem)())         \
BOOST_PP_TUPLE_ELEM(0, Elem)(void)                                  \
BOOST_PP_EXPR_IF(BOOST_PP_TUPLE_ELEM(1, Elem), const) {             \
    return Data.BOOST_PP_TUPLE_ELEM(0, Elem)();                     \
}

#define VE_IMPL_GEN_ITERATOR_TYPEDEF(Rep, Data, Elem)               \
using Elem = typename decltype(Data)::Elem;


#define ve_make_iteratable(member)                                  \
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
)                                                                   \
                                                                    \
auto reverse_view(void) {                                           \
    return ve::reverse_view { rbegin(), rend() };                   \
}                                                                   \
                                                                    \
auto reverse_view(void) const {                                     \
    return ve::reverse_view { crbegin(), crend() };                 \
}


namespace ve {
    template <typename It> class reverse_view {
    public:
        using value_type      = typename It::value_type;
        using difference_type = typename It::difference_type;
        using pointer         = typename It::pointer;
        using reference       = typename It::reference;
        
        reverse_view(It rbegin, It rend) : begin_it(rbegin), end_it(rend) {}
        
        It begin(void) { return begin_it; }
        It end(void) { return end_it; }
    private:
        It begin_it, end_it;
    };
}