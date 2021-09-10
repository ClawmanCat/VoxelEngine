#pragma once

#include <VoxelEngine/core/typedef/scalar.hpp>


#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#define GLM_SWIZZLE_RGBA
#define GLM_FORCE_SIZE_T_LENGTH

#if VE_GRAPHICS_API == vulkan
    #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <boost/preprocessor.hpp>


namespace ve {
    namespace defs {
        // List of types that have vector aliases.
        #define ve_impl_scalar_types ((i8, b))((u8, ub))((i16, s))((u16, us))((i32, i))((u32, ui))((i64, l))((u64, ul))((f32, f))((f64, d))

        // Given a tuple E (type, suffix), and a template D generates a quaternion alias for the type.
        #define ve_impl_def_for_t(R, D, E) using BOOST_PP_CAT(D, BOOST_PP_TUPLE_ELEM(1, E)) = D<BOOST_PP_TUPLE_ELEM(0, E)>;


        template <std::size_t N, typename T> using vec = glm::vec<N, T>;
        template <typename T> using vec2 = vec<2, T>;
        template <typename T> using vec3 = vec<3, T>;
        template <typename T> using vec4 = vec<4, T>;


        // Given a tuple E (type, suffix), generates vector aliases for that type.
        #define ve_impl_vec_defs_for_t(R, D, E)                                                 \
        using BOOST_PP_CAT(vec2, BOOST_PP_TUPLE_ELEM(1, E)) = vec2<BOOST_PP_TUPLE_ELEM(0, E)>;  \
        using BOOST_PP_CAT(vec3, BOOST_PP_TUPLE_ELEM(1, E)) = vec3<BOOST_PP_TUPLE_ELEM(0, E)>;  \
        using BOOST_PP_CAT(vec4, BOOST_PP_TUPLE_ELEM(1, E)) = vec4<BOOST_PP_TUPLE_ELEM(0, E)>;

        BOOST_PP_SEQ_FOR_EACH(ve_impl_vec_defs_for_t, _, ve_impl_scalar_types);


        template <std::size_t W, std::size_t H, typename T> using mat = glm::mat<W, H, T>;
        template <typename T> using mat2 = mat<2, 2, T>;
        template <typename T> using mat3 = mat<3, 3, T>;
        template <typename T> using mat4 = mat<4, 4, T>;


        #define ve_impl_square_mat(E, T, S) using BOOST_PP_SEQ_CAT((mat)(E)(S)) = mat<E, E, T>;

        // Given a tuple size (W, H), a type T and a suffix S, generates a matrix alias for that type.
        #define ve_impl_mat_defs_for_size(W, H, T, S)                                           \
        using BOOST_PP_SEQ_CAT((mat)(W)(x)(H)(S)) = mat<W, H, T>;                               \
                                                                                                \
        BOOST_PP_REMOVE_PARENS(BOOST_PP_EXPR_IF(                                                \
            BOOST_PP_EQUAL(W, H),                                                               \
            (ve_impl_square_mat(W, T, S))                                                       \
        ))

        // Calls the above macro, mapping the sequence E ((W)(H)((T, S))) to its arguments.
        #define ve_impl_mat_defs_for_size_wrapper(R, D, E)                                      \
        ve_impl_mat_defs_for_size(                                                              \
            BOOST_PP_SEQ_ELEM(0, E),                                                            \
            BOOST_PP_SEQ_ELEM(1, E),                                                            \
            BOOST_PP_TUPLE_ELEM(0, BOOST_PP_SEQ_ELEM(2, E)),                                    \
            BOOST_PP_TUPLE_ELEM(1, BOOST_PP_SEQ_ELEM(2, E))                                     \
        )

        #define ve_impl_mat_sizes (2)(3)(4)
        #define ve_impl_seq_wrap(R, S) (S)

        // Generates the cartesian product Widths x Heights x Types
        #define ve_impl_mat_size_seq BOOST_PP_SEQ_FOR_EACH_PRODUCT(ve_impl_seq_wrap, (ve_impl_mat_sizes)(ve_impl_mat_sizes)(ve_impl_scalar_types))

        BOOST_PP_SEQ_FOR_EACH(ve_impl_mat_defs_for_size_wrapper, _, ve_impl_mat_size_seq)


        template <typename T> using quat = glm::qua<T>;
        BOOST_PP_SEQ_FOR_EACH(ve_impl_def_for_t, quat, ve_impl_scalar_types)


        template <typename T> struct rect2 {
            vec2<T> top_left, bottom_right;

            vec2<T> size(void) const { return bottom_right - top_left; }
        };

        BOOST_PP_SEQ_FOR_EACH(ve_impl_def_for_t, rect2, ve_impl_scalar_types)
    };

    using namespace defs;
}