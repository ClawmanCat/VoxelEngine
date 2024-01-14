#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>

#include <iterator>
#include <cstddef>
#include <type_traits>


#define VE_IMPL_ITERATOR_TYPEDEFS(Iterator, ConstIterator, EnableIterator, EnableReverseIterator)                                                                   \
BOOST_PP_EXPR_IF(                                                                                                                                                   \
    EnableIterator,                                                                                                                                                 \
    using iterator          = Iterator;                                                                                                                             \
    using const_iterator    = ConstIterator;                                                                                                                        \
)                                                                                                                                                                   \
                                                                                                                                                                    \
BOOST_PP_EXPR_IF(                                                                                                                                                   \
    EnableReverseIterator,                                                                                                                                          \
    using reverse_iterator       = std::reverse_iterator<iterator>;                                                                                                 \
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;                                                                                           \
)


#define VE_IMPL_CONTAINER_TYPEDEFS(ValueType, AddConst, AddReference, AddPointer, Iterator, ConstIterator, EnableIterator, EnableReverseIterator)                   \
using value_type       = ValueType;                                                                                                                                 \
using const_value_type = AddConst<value_type>;                                                                                                                      \
using reference        = AddReference<value_type>;                                                                                                                  \
using const_reference  = AddReference<const_value_type>;                                                                                                            \
using pointer          = AddPointer<value_type>;                                                                                                                    \
using const_pointer    = AddPointer<const_value_type>;                                                                                                              \
using size_type        = std::size_t;                                                                                                                               \
using difference_type  = std::ptrdiff_t;                                                                                                                            \
                                                                                                                                                                    \
VE_IMPL_ITERATOR_TYPEDEFS(Iterator, ConstIterator, EnableIterator, EnableReverseIterator)


#define VE_CONTAINER_TYPEDEFS(ValueType) VE_IMPL_CONTAINER_TYPEDEFS(ValueType, std::add_const_t, std::add_lvalue_reference_t, std::add_pointer_t, void, void, VE_FALSE, VE_FALSE)
#define VE_CONTAINER_ITERATOR_TYPEDEFS(ValueType, Iterator, ConstIterator, Reversible) VE_IMPL_CONTAINER_TYPEDEFS(ValueType, std::add_const_t, std::add_lvalue_reference_t, std::add_pointer_t, Iterator, ConstIterator, VE_TRUE, Reversible)

#define VE_CONTAINER_TYPEDEFS_RP(ValueType, AddReference, AddPointer) VE_IMPL_CONTAINER_TYPEDEFS(ValueType, std::add_const_t, AddReference, AddPointer, void, void, VE_FALSE, VE_FALSE)
#define VE_CONTAINER_ITERATOR_TYPEDEFS_RP(ValueType, AddReference, AddPointer, Iterator, ConstIterator, Reversible) VE_IMPL_CONTAINER_TYPEDEFS(ValueType, std::add_const_t, AddReference, AddPointer, Iterator, ConstIterator, VE_TRUE, Reversible)


#define VE_IMPL_ITERATOR_FN(Iterator, Fn, KwConst, ArgTuple) \
[[nodiscard]] Iterator Fn(void) KwConst { return Iterator ArgTuple; }


#define VE_IMPL_CONTAINER_BEGIN_END(BeginConstructorTuple, EndConstructorTuple, Reversible)                                                                         \
VE_IMPL_ITERATOR_FN(iterator,       begin,  /* not const */, BeginConstructorTuple)                                                                                 \
VE_IMPL_ITERATOR_FN(const_iterator, begin,  const,           BeginConstructorTuple)                                                                                 \
VE_IMPL_ITERATOR_FN(const_iterator, cbegin, const,           BeginConstructorTuple)                                                                                 \
VE_IMPL_ITERATOR_FN(iterator,       end,    /* not const */, EndConstructorTuple)                                                                                   \
VE_IMPL_ITERATOR_FN(const_iterator, end,    const,           EndConstructorTuple)                                                                                   \
VE_IMPL_ITERATOR_FN(const_iterator, cend,   const,           EndConstructorTuple)                                                                                   \
                                                                                                                                                                    \
BOOST_PP_EXPR_IF(                                                                                                                                                   \
    Reversible,                                                                                                                                                     \
    VE_IMPL_ITERATOR_FN(reverse_iterator,       rbegin,  /* not const */, ( iterator       EndConstructorTuple   ))                                                 \
    VE_IMPL_ITERATOR_FN(const_reverse_iterator, rbegin,  const,           ( const_iterator EndConstructorTuple   ))                                                 \
    VE_IMPL_ITERATOR_FN(const_reverse_iterator, crbegin, const,           ( const_iterator EndConstructorTuple   ))                                                 \
    VE_IMPL_ITERATOR_FN(reverse_iterator,       rend,    /* not const */, ( iterator       BeginConstructorTuple ))                                                 \
    VE_IMPL_ITERATOR_FN(const_reverse_iterator, rend,    const,           ( const_iterator BeginConstructorTuple ))                                                 \
    VE_IMPL_ITERATOR_FN(const_reverse_iterator, crend,   const,           ( const_iterator BeginConstructorTuple ))                                                 \
)


#define VE_CONTAINER_BEGIN_END(BeginConstructorTuple, EndConstructorTuple)         VE_IMPL_CONTAINER_BEGIN_END(BeginConstructorTuple, EndConstructorTuple, VE_TRUE)
#define VE_ONE_WAY_CONTAINER_BEGIN_END(BeginConstructorTuple, EndConstructorTuple) VE_IMPL_CONTAINER_BEGIN_END(BeginConstructorTuple, EndConstructorTuple, VE_FALSE)