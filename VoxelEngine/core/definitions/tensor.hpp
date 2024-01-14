#pragma once

#include <VoxelEngine/core/definitions/scalar.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/vec1.hpp>
#include <glm/gtc/quaternion.hpp>

#include <boost/preprocessor.hpp>


// A sequence of tuples (typename, suffix) for every scalar type.
#define VE_IMPL_SCALAR_TYPES \
((i8, b))((u8, ub))((i16, s))((u16, us))((i32, i))((u32, ui))((i64, l))((u64, ul))((f32, f))((f64, d))((bool, bl))


namespace ve {
    template <typename T> struct glm_traits {
        constexpr static inline bool is_vector = false;
        constexpr static inline bool is_matrix = false;
        constexpr static inline bool is_quat   = false;
    };

    template <std::size_t N, typename T> struct glm_traits<glm::vec<N, T>> {
        constexpr static inline bool is_vector = true;
        constexpr static inline bool is_matrix = false;
        constexpr static inline bool is_quat   = false;
    
        constexpr static inline std::size_t size = N;
        using value_type = T;
    };
    
    template <std::size_t W, std::size_t H, typename T> struct glm_traits<glm::mat<W, H, T>> {
        constexpr static inline bool is_vector = false;
        constexpr static inline bool is_matrix = true;
        constexpr static inline bool is_quat   = false;
    
        constexpr static inline std::size_t width  = W;
        constexpr static inline std::size_t height = H;
        using value_type = T;
    };
    
    template <typename T> struct glm_traits<glm::qua<T>> {
        constexpr static inline bool is_vector = false;
        constexpr static inline bool is_matrix = false;
        constexpr static inline bool is_quat   = true;

        using value_type = T;
    };


    // Vector Definitions
    template <std::size_t N, typename T> using vec = glm::vec<N, T>;

    template <typename T>                concept glm_vector         = glm_traits<T>::is_vector;
    template <typename T, std::size_t N> concept glm_vector_of_size = glm_traits<T>::is_vector && glm_traits<T>::size == N;


    #define VE_IMPL_VEC_DEF(R, D, E) using BOOST_PP_SEQ_CAT((vec)(D)(BOOST_PP_TUPLE_ELEM(1, E))) = BOOST_PP_CAT(vec, D)<BOOST_PP_TUPLE_ELEM(0, E)>;

    #define VE_IMPL_VEC_DEFS(Z, N, D)                                   \
    template <typename T> using vec##N = glm::vec<N, T>;                \
    BOOST_PP_SEQ_FOR_EACH(VE_IMPL_VEC_DEF, N, VE_IMPL_SCALAR_TYPES)

    BOOST_PP_REPEAT_FROM_TO(1, 5, VE_IMPL_VEC_DEFS, _)


    // Matrix Definitions
    template <std::size_t W, std::size_t H, typename T> using mat = glm::mat<W, H, T>;
    template <typename T> using mat2 = glm::mat<2, 2, T>;
    template <typename T> using mat3 = glm::mat<3, 3, T>;
    template <typename T> using mat4 = glm::mat<4, 4, T>;

    template <typename T>                               concept glm_matrix             = glm_traits<T>::is_matrix;
    template <typename T, std::size_t W>                concept glm_matrix_with_width  = glm_traits<T>::is_matrix && glm_traits<T>::width  == W;
    template <typename T, std::size_t H>                concept glm_matrix_with_height = glm_traits<T>::is_matrix && glm_traits<T>::height == H;
    template <typename T, std::size_t W, std::size_t H> concept glm_matrix_with_size   = glm_traits<T>::is_matrix && glm_traits<T>::width  == W && glm_traits<T>::height == H;


    #define VE_IMPL_MAT_SIZES (2)(3)(4)

    #define VE_IMPL_MAT_DEF(W, H, T, Sfx)                               \
    using BOOST_PP_SEQ_CAT((mat)(W)(x)(H)(Sfx)) = mat<W, H, T>;         \
                                                                        \
    BOOST_PP_REMOVE_PARENS(BOOST_PP_EXPR_IF(                            \
        BOOST_PP_EQUAL(W, H),                                           \
        (using BOOST_PP_SEQ_CAT((mat)(W)(Sfx)) = mat<                   \
            W BOOST_PP_COMMA()                                          \
            H BOOST_PP_COMMA()                                          \
            T                                                           \
        >;)                                                             \
    ))


    #define VE_IMPL_MAT_DEF_UNPACK(R, Elems)                            \
    VE_IMPL_MAT_DEF(                                                    \
        BOOST_PP_SEQ_ELEM(0, Elems),                                    \
        BOOST_PP_SEQ_ELEM(1, Elems),                                    \
        BOOST_PP_TUPLE_ELEM(0, BOOST_PP_SEQ_ELEM(2, Elems)),            \
        BOOST_PP_TUPLE_ELEM(1, BOOST_PP_SEQ_ELEM(2, Elems))             \
    )

    BOOST_PP_SEQ_FOR_EACH_PRODUCT(VE_IMPL_MAT_DEF_UNPACK, (VE_IMPL_MAT_SIZES)(VE_IMPL_MAT_SIZES)(VE_IMPL_SCALAR_TYPES))


    // Quaternion Definitions
    template <typename T> using quat = glm::qua<T>;
    template <typename T> concept glm_quaternion = glm_traits<T>::is_quat;

    using quatf = quat<f32>;
    using quatd = quat<f64>;
}