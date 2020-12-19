#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>
#include <boost/container_hash/hash.hpp>

#include <cstddef>


#define VE_IMPL_HASH_SINGLE(val)                                \
std::hash<decltype(obj.val)>{}(obj.val)


#define VE_IMPL_HASH_COMBINE(Rep, Data, Elem) boost::hash_combine(seed, VE_IMPL_HASH_SINGLE(Elem));

#define VE_IMPL_HASH_MULTIPLE(...)                              \
[&](){                                                          \
    std::size_t seed = 0;                                       \
                                                                \
    BOOST_PP_SEQ_FOR_EACH(                                      \
        VE_IMPL_HASH_COMBINE,                                   \
        _,                                                      \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                   \
    );                                                          \
                                                                \
    return seed;                                                \
}()


#define ve_make_hashable(cls, ...)                              \
template <> struct std::hash<cls> {                             \
    std::size_t operator()(const cls& obj) const noexcept {     \
        return BOOST_PP_IF(                                     \
            BOOST_PP_EQUAL(                                     \
                BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),            \
                1                                               \
            ),                                                  \
            VE_IMPL_HASH_SINGLE,                                \
            VE_IMPL_HASH_MULTIPLE                               \
        )(__VA_ARGS__);                                         \
    }                                                           \
};