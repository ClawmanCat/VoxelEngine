#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/declval.hpp>
#include <VoxelEngine/utility/meta/pack/create_pack.hpp>

#include <boost/preprocessor.hpp>

#include <tuple>


#if defined(VE_IDE_PASS)
    // Use a small decomposition limit for IDEs as selecting the correct overload may cause large amounts of lag.
    #define VE_DECOMPOSER_LIMIT 4
#elif !defined(VE_DECOMPOSER_LIMIT)
    // May be set by user.
    #define VE_DECOMPOSER_LIMIT 64
#endif


#define VE_IMPL_STRUCTURED_BINDING_ELEM(Z, N, D) ( BOOST_PP_CAT(e, N) )
#define VE_IMPL_DECLTYPE_ELEM(Z, N, D) ( decltype(BOOST_PP_CAT(e, N)) )

#define VE_IMPL_STRUCTURED_BINDING(N) BOOST_PP_SEQ_ENUM(BOOST_PP_REPEAT(N, VE_IMPL_STRUCTURED_BINDING_ELEM, _))
#define VE_IMPL_DECLTYPE_SEQ(N) BOOST_PP_SEQ_ENUM(BOOST_PP_REPEAT(N, VE_IMPL_DECLTYPE_ELEM, _))


/** Generates a struct to decompose a class with N decomposable members. */
#define VE_IMPL_DECOMPOSE_N(Z, N, D)                                                                                            \
template <typename T> requires requires { [] { const auto& [ VE_IMPL_STRUCTURED_BINDING(N) ] = ve::meta::declval<T>(); }; }     \
struct n_element_decomposer<T> {                                                                                                \
    constexpr static inline bool is_decomposable = true;                                                                        \
    constexpr static inline std::size_t size = N;                                                                               \
                                                                                                                                \
    constexpr static auto reference_as_tuple(const T& value) {                                                                  \
        const auto& [ VE_IMPL_STRUCTURED_BINDING(N) ] = value;                                                                  \
        return std::forward_as_tuple(VE_IMPL_STRUCTURED_BINDING(N));                                                            \
    }                                                                                                                           \
                                                                                                                                \
    constexpr static auto reference_as_tuple(T& value) {                                                                        \
        auto& [ VE_IMPL_STRUCTURED_BINDING(N) ] = value;                                                                        \
        return std::forward_as_tuple(VE_IMPL_STRUCTURED_BINDING(N));                                                            \
    }                                                                                                                           \
                                                                                                                                \
    constexpr static auto copy_as_tuple(const T& value) {                                                                       \
        auto [ VE_IMPL_STRUCTURED_BINDING(N) ] = value;                                                                         \
        return std::tuple< VE_IMPL_DECLTYPE_SEQ(N) > { VE_IMPL_STRUCTURED_BINDING(N) };                                         \
    }                                                                                                                           \
                                                                                                                                \
    using decomposed_type = T;                                                                                                  \
    using member_types    = ve::meta::create_pack::from_template<decltype(copy_as_tuple(std::declval<T>()))>;                   \
};


namespace ve::detail {
    template <typename T> struct n_element_decomposer {
        constexpr static inline bool is_decomposable = false;
    };


    template <typename T> requires std::is_empty_v<T>
    struct n_element_decomposer<T> {
        constexpr static inline bool is_decomposable = true;
        constexpr static inline std::size_t size = 0;
    };


    BOOST_PP_REPEAT_FROM_TO(
        1, VE_DECOMPOSER_LIMIT,
        VE_IMPL_DECOMPOSE_N,
        _
    );


    template <typename T> concept binding_decomposable = detail::n_element_decomposer<T>::is_decomposable;
}
