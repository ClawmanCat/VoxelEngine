#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/types.hpp>
#include <VoxelEngine/utils/meta/traits/glm_traits.hpp>
#include <VoxelEngine/utils/meta/traits/any_of.hpp>
#include <VoxelEngine/utils/meta/traits/always_false.hpp>
#include <VoxelEngine/graphics/render/texture/texture.hpp>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <type_traits>


namespace ve::generic_uniform {
    namespace detail {
        template <typename T> void set_uniform_impl(GLint index, const T& value, u32& state) {
            static_assert(meta::always_false_v<T>, "Unsupported uniform type.");
        }
        
        
        template <typename T> requires std::is_same_v<T, texture>
        inline void set_uniform_impl(GLint index, const T& value, u32& state) {
            VE_DEBUG_ONLY(
                GLint num_tex_units = 0;
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_tex_units);
                
                VE_ASSERT(num_tex_units > (i32) state);
            );
            
            
            glActiveTexture(GL_TEXTURE0 + state);
            glBindTexture(GL_TEXTURE_2D, value.id);
            glUniform1i(index, state);
            
            state++;
        }
        
        
        // Scalar types.
        #define VE_IMPL_SCALAR_TYPEMAP(gl_suffix, gl_type, ...) \
        if constexpr (meta::is_any_of_v<T, __VA_ARGS__>) glUniform1##gl_suffix(index, (gl_type) value)
        
        
        template <typename T> requires (meta::scalar_types::template contains<T>())
        inline void set_uniform_impl(GLint index, const T& value, u32& state) {
            VE_IMPL_SCALAR_TYPEMAP(i,  GLint,    i8, i16, i32, i64);
            VE_IMPL_SCALAR_TYPEMAP(ui, GLuint,   u8, u16, u32, u64);
            VE_IMPL_SCALAR_TYPEMAP(f,  GLfloat,  f32);
            VE_IMPL_SCALAR_TYPEMAP(d,  GLdouble, f64);
        }
        
        
        // Vector types.
        #define VE_IMPL_VECTOR_TYPEMAP_FOR_SIZE(Rep, Data, Elem)                                \
        if constexpr (meta::glm_traits<T>::size == Elem && meta::is_any_of_v<                   \
            typename meta::glm_traits<T>::data_type,                                            \
            BOOST_PP_SEQ_ENUM(BOOST_PP_TUPLE_ELEM(2, Data))                                     \
        >) {                                                                                    \
            BOOST_PP_SEQ_CAT((glUniform)(Elem)(BOOST_PP_TUPLE_ELEM(0, Data))(v)) (              \
                index,                                                                          \
                1,                                                                              \
                glm::value_ptr((vec<Elem, BOOST_PP_TUPLE_ELEM(1, Data)>) value)                 \
            );                                                                                  \
        }
        
        #define VE_IMPL_VECTOR_TYPEMAP(gl_suffix, gl_type, ...)                                 \
        BOOST_PP_SEQ_FOR_EACH(                                                                  \
            VE_IMPL_VECTOR_TYPEMAP_FOR_SIZE,                                                    \
            (gl_suffix, gl_type, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)),                        \
            (2)(3)(4)                                                                           \
        )
        
        template <typename T> requires meta::glm_traits<T>::is_vector
        inline void set_uniform_impl(GLint index, const T& value, u32& state) {
            VE_IMPL_VECTOR_TYPEMAP(i,  GLint,    i8, i16, i32, i64);
            VE_IMPL_VECTOR_TYPEMAP(ui, GLuint,   u8, u16, u32, u64);
            VE_IMPL_VECTOR_TYPEMAP(f,  GLfloat,  f32);
            VE_IMPL_VECTOR_TYPEMAP(d,  GLdouble, f64);
        }
        
        
        // Matrix types.
        #define VE_IMPL_MATRIX_TYPEMAP_FOR_SIZE(Rep, Data, Elem)                                \
        if constexpr (                                                                          \
            meta::glm_traits<T>::is_shape(BOOST_PP_STRINGIZE(Elem)) &&                          \
            meta::is_any_of_v<                                                                  \
                typename meta::glm_traits<T>::data_type,                                        \
                BOOST_PP_SEQ_ENUM(BOOST_PP_TUPLE_ELEM(2, Data))                                 \
            >                                                                                   \
        ) {                                                                                     \
            BOOST_PP_SEQ_CAT((glUniformMatrix)(Elem)(BOOST_PP_TUPLE_ELEM(0, Data))(v)) (        \
                index,                                                                          \
                1,                                                                              \
                GL_FALSE,                                                                       \
                glm::value_ptr((BOOST_PP_CAT(mat, Elem<BOOST_PP_TUPLE_ELEM(1, Data)>)) value)   \
            );                                                                                  \
        }

        #define VE_IMPL_MATRIX_TYPEMAP(gl_suffix, gl_type, ...)                                 \
        BOOST_PP_SEQ_FOR_EACH(                                                                  \
            VE_IMPL_MATRIX_TYPEMAP_FOR_SIZE,                                                    \
            (gl_suffix, gl_type, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)),                        \
            (2)(3)(4)(2x3)(2x4)(3x2)(3x4)(4x2)(4x3)                                             \
        )
        
        template <typename T> requires meta::glm_traits<T>::is_matrix
        inline void set_uniform_impl(GLint index, const T& value, u32& state) {
            VE_IMPL_MATRIX_TYPEMAP(f,  GLfloat,  f32);
            VE_IMPL_MATRIX_TYPEMAP(d,  GLdouble, f64);
        }
    }
    
    
    // State is required to decide what texture unit to bind each texture to.
    // Initialize it to zero before setting the uniforms associated with a single glDraw* call.
    template <typename T> inline void set_uniform(GLuint program, const char* name, const T& value, u32& state) {
        return detail::set_uniform_impl(
            glGetUniformLocation(program, name),
            value,
            state
        );
    }
}