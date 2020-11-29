#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/glm_traits.hpp>

#include <GL/glew.h>
#include <boost/preprocessor.hpp>

#include <string_view>
#include <cstddef>
#include <type_traits>



#define VE_IMPL_GEN_VERTEX_ATTRIB_INFO(cls, member)                                             \
((ve::vertex_attribute {                                                                        \
    #member,                                                                                    \
    sizeof(cls::member),                                                                        \
    offsetof(cls, member),                                                                      \
    ve::meta::glm_traits<decltype(cls::member)>::size,                                          \
    ve::detail::get_gl_type<typename ve::meta::glm_traits<decltype(cls::member)>::data_type>()  \
}))


#define VE_IMPL_GEN_VERTEX_ATTRIB_INFO_WRAPPER(rep, cls, member)                                \
VE_IMPL_GEN_VERTEX_ATTRIB_INFO(cls, member)


#define VE_GEN_ATTRIB_FN(cls, ...)                                                              \
constexpr static auto get_attributes(void) {                                                    \
    std::array attributes = {                                                                   \
        BOOST_PP_SEQ_ENUM(                                                                      \
            BOOST_PP_SEQ_FOR_EACH(                                                              \
                VE_IMPL_GEN_VERTEX_ATTRIB_INFO_WRAPPER,                                         \
                cls,                                                                            \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                           \
            )                                                                                   \
        )                                                                                       \
    };                                                                                          \
                                                                                                \
    return attributes;                                                                          \
}


namespace ve {
    struct vertex_attribute {
        const char* name;
        std::size_t size, offset, count;
        GLenum type;
    };
    
    
    namespace detail {
        template <typename T> constexpr inline GLenum get_gl_type(void) {
            GLenum type = GL_INVALID_ENUM;
            
            if constexpr (std::is_same_v<T, i8 >) type = GL_BYTE;
            if constexpr (std::is_same_v<T, u8 >) type = GL_UNSIGNED_BYTE;
            if constexpr (std::is_same_v<T, i16>) type = GL_SHORT;
            if constexpr (std::is_same_v<T, u16>) type = GL_UNSIGNED_SHORT;
            if constexpr (std::is_same_v<T, i32>) type = GL_INT;
            if constexpr (std::is_same_v<T, u32>) type = GL_UNSIGNED_INT;
            if constexpr (std::is_same_v<T, f32>) type = GL_FLOAT;
            if constexpr (std::is_same_v<T, f64>) type = GL_DOUBLE;
    
            return type;
        }
    }
}