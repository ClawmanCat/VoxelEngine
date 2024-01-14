#pragma once

#include <boost/preprocessor.hpp>


#define VE_IMPL_PRAGMA(...) _Pragma(#__VA_ARGS__)

#define VE_IMPL_PG_DISABLE(R, D, E)                         \
VE_IMPL_PRAGMA(E diagnostic push)                           \
VE_IMPL_PRAGMA(E diagnostic ignored #D)

#define VE_IMPL_PG_ENABLE(R, D, E) VE_IMPL_PRAGMA(E diagnostic pop)


/** Mutes the given warning for the given compilers during the provided statement. */
#define VE_MUTE_WARNING(CompilerTuple, Warning, ...)        \
BOOST_PP_SEQ_FOR_EACH(                                      \
    VE_IMPL_PG_DISABLE,                                     \
    Warning,                                                \
    BOOST_PP_TUPLE_TO_SEQ(CompilerTuple)                    \
)                                                           \
                                                            \
__VA_ARGS__                                                 \
                                                            \
BOOST_PP_SEQ_FOR_EACH(                                      \
    VE_IMPL_PG_ENABLE,                                      \
    Warning,                                                \
    BOOST_PP_TUPLE_TO_SEQ(CompilerTuple)                    \
)