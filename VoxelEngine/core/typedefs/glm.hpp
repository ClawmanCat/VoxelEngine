#pragma once

#include <VoxelEngine/core/preprocessor.hpp>
#include <VoxelEngine/core/typedefs/scalar.hpp>

#include <boost/preprocessor.hpp>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
#pragma clang diagnostic ignored "-Wformat-nonliteral"

#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#define GLM_SWIZZLE_RGBA
#define GLM_FORCE_SIZE_T_LENGTH
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

#pragma clang diagnostic pop


// Sequence of all scalar types usable in GLM tensors. Contains pairs of (typename, glm type suffix).
#define VE_IMPL_SCALAR_TYPE_SEQ \
((i8, b))((u8, ub))((i16, s))((u16, us))((i32, i))((u32, ui))((i64, l))((u64, ul))((f32, f))((f64, d))((bool, bl))


// Given a matrix shape as a pair, constructs a class agnostic matrix typedef for that shape.
#define VE_IMPL_GEN_GENERIC_MAT(Rep, Data, Elem)                                                            \
template <typename T> using                                                                                 \
BOOST_PP_SEQ_CAT((mat)(BOOST_PP_TUPLE_ELEM(0, Elem))(x)(BOOST_PP_TUPLE_ELEM(1, Elem)))                      \
= mat<BOOST_PP_TUPLE_ELEM(0, Elem), BOOST_PP_TUPLE_ELEM(1, Elem), T>;


// Given a type, constructs a typedef for every different vector length for vectors of that type.
#define VE_IMPL_VEC_TYPES(Type)                                                                             \
using BOOST_PP_CAT(vec2, BOOST_PP_TUPLE_ELEM(1, Type)) = vec2<BOOST_PP_TUPLE_ELEM(0, Type)>;                \
using BOOST_PP_CAT(vec3, BOOST_PP_TUPLE_ELEM(1, Type)) = vec3<BOOST_PP_TUPLE_ELEM(0, Type)>;                \
using BOOST_PP_CAT(vec4, BOOST_PP_TUPLE_ELEM(1, Type)) = vec4<BOOST_PP_TUPLE_ELEM(0, Type)>;

// Wrapper for the above macro for use with BOOST_PP_SEQ_FOR_EACH.
#define VE_IMPL_GEN_VEC_TYPES(R, D, E) VE_IMPL_VEC_TYPES(E)


// Given a type (E), constructs a typedef for the quaternion of that type.
// (Yes, glm uses 'qua' and not 'quat'.)
#define VE_IMPL_GEN_QUAT_TYPE(R, D, E) \
using BOOST_PP_CAT(quat, BOOST_PP_TUPLE_ELEM(1, E)) = glm::qua<BOOST_PP_TUPLE_ELEM(0, E)>;


// Given a type (T) and a size (S), constructs a square matrix typedef of that type and size.
#define VE_IMPL_SQUARE_MAT_TYPE(S, T)                                                                       \
using BOOST_PP_CAT(mat##S, BOOST_PP_TUPLE_ELEM(1, T)) = glm::mat<S, S, BOOST_PP_TUPLE_ELEM(0, T)>;


// Given a type (T) and a size (C x R), constructs a matrix typedef of that type and size.
// If C == R, VE_SQUARE_MAT_TYPE is also called.
#define VE_IMPL_MAT_TYPE(C, R, T)                                                                           \
using BOOST_PP_CAT(mat##C##x##R, BOOST_PP_TUPLE_ELEM(1, T)) = glm::mat<C, R, BOOST_PP_TUPLE_ELEM(0, T)>;    \
                                                                                                            \
BOOST_PP_IF(                                                                                                \
    BOOST_PP_EQUAL(C, R),                                                                                   \
    VE_IMPL_SQUARE_MAT_TYPE,                                                                                \
    VE_EMPTY                                                                                                \
)(C, T)


// Given a width and a type, generate matrix typedefs for all heights with that width and type.
#define VE_IMPL_MAT_TYPES_C(C, T)                                                                           \
VE_IMPL_MAT_TYPE(C, 2, T); VE_IMPL_MAT_TYPE(C, 3, T); VE_IMPL_MAT_TYPE(C, 4, T);

// Given a type, generate matrix typedefs for all sizes with that type.
#define VE_IMPL_MAT_TYPES(T)                                                                                \
VE_IMPL_MAT_TYPES_C(2, T); VE_IMPL_MAT_TYPES_C(3, T); VE_IMPL_MAT_TYPES_C(4, T);

// Wrapper for the above macro for use with BOOST_PP_SEQ_FOR_EACH.
#define VE_IMPL_GEN_MAT_TYPES(R, D, E) VE_IMPL_MAT_TYPES(E)


namespace ve {
    // Class Agnostic Typedefs
    template <std::size_t R, typename T> using vec = glm::vec<R, T>;
    template <std::size_t C, std::size_t R, typename T> using mat = glm::mat<C, R, T>;
    
    template <typename T> using vec2 = vec<2, T>;
    template <typename T> using vec3 = vec<3, T>;
    template <typename T> using vec4 = vec<4, T>;
    
    template <typename T> using mat2 = mat<2, 2, T>;
    template <typename T> using mat3 = mat<3, 3, T>;
    template <typename T> using mat4 = mat<4, 4, T>;
    
    BOOST_PP_SEQ_FOR_EACH(
        VE_IMPL_GEN_GENERIC_MAT,
        _,
        ((2, 2))((2, 3))((2, 4))((3, 2))((3, 3))((3, 4))((4, 2))((4, 3))((4, 4))
    );
    
    // Vector typedefs for all types in VE_SCALAR_TYPE_SEQ.
    BOOST_PP_SEQ_FOR_EACH(
        VE_IMPL_GEN_VEC_TYPES,
        _,
        VE_IMPL_SCALAR_TYPE_SEQ
    );
    
    // Quaternion typedefs for all types in VE_SCALAR_TYPE_SEQ.
    BOOST_PP_SEQ_FOR_EACH(
        VE_IMPL_GEN_QUAT_TYPE,
        _,
        VE_IMPL_SCALAR_TYPE_SEQ
    );
    
    // Matrix typedefs for all types in VE_SCALAR_TYPE_SEQ.
    BOOST_PP_SEQ_FOR_EACH(
        VE_IMPL_GEN_MAT_TYPES,
        _,
        VE_IMPL_SCALAR_TYPE_SEQ
    );
    
    
    // Comparison operator overloads.
    template <typename T, std::size_t N>
    constexpr inline auto operator<(const vec<N, T>& a, const vec<N, T>& b) {
        return glm::lessThan(a, b);
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator>(const vec<N, T>& a, const vec<N, T>& b) {
        return glm::greaterThan(a, b);
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator<=(const vec<N, T>& a, const vec<N, T>& b) {
        return glm::lessThanEqual(a, b);
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator>=(const vec<N, T>& a, const vec<N, T>& b) {
        return glm::greaterThanEqual(a, b);
    }
    
    
    template <typename T, std::size_t N>
    constexpr inline auto operator<(const vec<N, T>& a, const T& b) {
        return glm::lessThan(a, glm::vec<N, T>(b));
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator>(const vec<N, T>& a, const T& b) {
        return glm::greaterThan(a, glm::vec<N, T>(b));
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator<=(const vec<N, T>& a, const T& b) {
        return glm::lessThanEqual(a, glm::vec<N, T>(b));
    }
    
    template <typename T, std::size_t N>
    constexpr inline auto operator>=(const vec<N, T>& a, const T& b) {
        return glm::greaterThanEqual(a, glm::vec<N, T>(b));
    }
}