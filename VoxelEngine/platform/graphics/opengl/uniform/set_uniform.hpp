#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/type_categories.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/utility/traits/any_of.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <type_traits>


namespace ve::graphics {
    namespace detail {
        template <typename T> void set_uniform_impl(GLint index, const T& value, context& ctx) {
            static_assert(meta::always_false_v<T>, "Unsupported uniform type.");
        }
        
        
        // Textures
        template <typename T> requires std::is_same_v<T, shared<texture>>
        inline void set_uniform_impl(GLint index, const T& value, context& ctx) {
            const static u32 texture_unit_limit = [](){
                GLint num_tex_units = 0;
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_tex_units);
                
                return (u32) num_tex_units;
            }();
    
            
            VE_ASSERT(
                ctx.next_texture_unit < texture_unit_limit,
                "Hardware texture limit exceeded. Try merging some textures into an atlas, "
                "or try using a larger atlas size if your textures are already merged."
            );
            
            glActiveTexture(GL_TEXTURE0 + ctx.next_texture_unit);
            glBindTexture(GL_TEXTURE_2D, value->get_id());
            glUniform1i(index, ctx.next_texture_unit);
        
            ctx.next_texture_unit++;
        }
        
        
        // Scalar types.
        #define VE_IMPL_SCALAR_TYPEMAP(gl_suffix, gl_type, ...) \
        if constexpr (meta::is_any_of_v<T, __VA_ARGS__>) glUniform1##gl_suffix(index, (gl_type) value)
        
        
        template <typename T> requires (meta::scalar_types::template contains<T>())
        inline void set_uniform_impl(GLint index, const T& value, context& ctx) {
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
        inline void set_uniform_impl(GLint index, const T& value, context& ctx) {
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
        inline void set_uniform_impl(GLint index, const T& value, context& ctx) {
            VE_IMPL_MATRIX_TYPEMAP(f,  GLfloat,  f32);
            VE_IMPL_MATRIX_TYPEMAP(d,  GLdouble, f64);
        }
    }
    
    
    template <typename T> inline void set_uniform(const char* name, const T& value, context& ctx) {
        GLint location = glGetUniformLocation(ctx.current_program, name);
        if (location == -1) return;
        
        detail::set_uniform_impl(location, value, ctx);
    }
}