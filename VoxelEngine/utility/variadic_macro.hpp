#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>


// Expands to element Index of __VA_ARGS__ if it has that many elements, otherwise expands to Value.
#define VE_IMPL_VARIADIC_DEFAULT(Index, Value, ...)                                                         \
BOOST_PP_IF(                                                                                                \
    BOOST_PP_GREATER_EQUAL(                                                                                 \
        Index,                                                                                              \
        BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                                 \
    ),                                                                                                      \
    Value,                                                                                                  \
    BOOST_PP_SEQ_ELEM(                                                                                      \
        BOOST_PP_MIN(                                                                                       \
            Index,                                                                                          \
            BOOST_PP_SUB(                                                                                   \
                BOOST_PP_SEQ_SIZE(                                                                          \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                   \
                ),                                                                                          \
                1                                                                                           \
            )                                                                                               \
        ),                                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                               \
    )                                                                                                       \
)


// Expands to all but the first N elements of __VA_ARGS__.
// Still works if N is more than the number of elements in __VA_ARGS__.
#define VE_IMPL_VARIADIC_REST(Index, ...)                                                                   \
BOOST_PP_REMOVE_PARENS(                                                                                     \
    BOOST_PP_EXPAND(                                                                                        \
        BOOST_PP_IF(                                                                                        \
            BOOST_PP_LESS(                                                                                  \
                BOOST_PP_MIN(                                                                               \
                    Index,                                                                                  \
                    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                     \
                ),                                                                                          \
                BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                         \
            ),                                                                                              \
            (                                                                                               \
                BOOST_PP_SEQ_ENUM(                                                                          \
                    BOOST_PP_SEQ_REST_N(                                                                    \
                        BOOST_PP_MIN(                                                                       \
                            Index,                                                                          \
                            BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                             \
                        ),                                                                                  \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
                    )                                                                                       \
                )                                                                                           \
            ),                                                                                              \
            ()                                                                                              \
        )                                                                                                   \
    )                                                                                                       \
)


