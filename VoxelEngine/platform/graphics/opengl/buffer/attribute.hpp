#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>

#include <GL/glew.h>
#include <boost/preprocessor.hpp>
#include <boost/pfr.hpp>

#include <string_view>
#include <type_traits>
#include <cstddef>


// Constructs a vertex_attribute object for the given class member.
#define VE_IMPL_GEN_VERTEX_ATTRIB_INFO(cls, member)                 \
((ve::graphics::vertex_attribute {                                  \
    #member,                                                        \
    sizeof(cls::member),                                            \
    offsetof(cls, member),                                          \
    ve::meta::glm_traits<decltype(cls::member)>::size,              \
    ve::graphics::detail::get_gl_type<                              \
        ve::meta::glm_traits<decltype(cls::member)>::data_type      \
    >()                                                             \
}))


// Wrapper for the above macro to call it using BOOST_PP_SEQ_FOR_EACH.
#define VE_IMPL_GEN_VERTEX_ATTRIB_INFO_WRAPPER(rep, cls, member)    \
VE_IMPL_GEN_VERTEX_ATTRIB_INFO(cls, member)


// Given a class and a set of members, generates a method which returns
// vertex_attribute objects for each member.
#define VE_GEN_ATTRIB_FN(cls, ...)                                  \
constexpr static auto get_attributes(void) {                        \
    std::array attributes = {                                       \
        BOOST_PP_SEQ_ENUM(                                          \
            BOOST_PP_SEQ_FOR_EACH(                                  \
                VE_IMPL_GEN_VERTEX_ATTRIB_INFO_WRAPPER,             \
                cls,                                                \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)               \
            )                                                       \
        )                                                           \
    };                                                              \
                                                                    \
    return attributes;                                              \
}


namespace ve::graphics {
    struct vertex_attribute {
        const char* name;
        std::size_t size, offset, count;
        GLenum type;
        
        ve_eq_comparable(vertex_attribute);
    };
    
    
    namespace detail {
        template <typename T> consteval inline GLenum get_gl_type(void) {
            GLenum type = GL_INVALID_ENUM;
    
            if constexpr (std::is_same_v<T, i8  >) type = GL_BYTE;
            if constexpr (std::is_same_v<T, u8  >) type = GL_UNSIGNED_BYTE;
            if constexpr (std::is_same_v<T, i16 >) type = GL_SHORT;
            if constexpr (std::is_same_v<T, u16 >) type = GL_UNSIGNED_SHORT;
            if constexpr (std::is_same_v<T, i32 >) type = GL_INT;
            if constexpr (std::is_same_v<T, u32 >) type = GL_UNSIGNED_INT;
            if constexpr (std::is_same_v<T, f32 >) type = GL_FLOAT;
            if constexpr (std::is_same_v<T, f64 >) type = GL_DOUBLE;
            if constexpr (std::is_same_v<T, bool>) type = GL_BOOL;
    
            return type;
        }
        
        
        template <typename T>
        inline vertex_attribute make_anonymous_attribute(std::size_t& offset_accumulator) {
            // If the current offset does not allow the placement of an object of type T,
            // account for the presence of padding bytes before the object.
            // Caller may check next_aligned_address(offset_accumulator, alignof(Parent)) == sizeof(Parent)
            // after calling this method for all class members to assert the struct in question is not packed.
            std::size_t padding = next_aligned_address(offset_accumulator, alignof(T)) - offset_accumulator;
            offset_accumulator += padding;
            
            return vertex_attribute {
                .name   = nullptr,
                .size   = sizeof(T),
                .offset = post_add(offset_accumulator, sizeof(T)),
                .count  = meta::glm_traits<T>::size,
                .type   = get_gl_type<meta::glm_traits<T>::data_type>()
            };
        }
        
        
        template <typename Cls>
        constexpr inline auto make_anonymous_attribute_array(void) {
            std::size_t offset_accumulator = 0;
    
            
            // Use boost PFR to get the type of each member and accumulate their sizes (+ padding)
            // to get the offsets of consecutive elements.
            auto result = [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
                return std::array {
                    make_anonymous_attribute<
                        boost::pfr::tuple_element_t<Indices, Cls>
                    >(offset_accumulator)...
                };
            }(std::make_index_sequence<boost::pfr::tuple_size_v<Cls>>());
            
            
            // Check for padding issues.
            VE_ASSERT(
                next_aligned_address(offset_accumulator, alignof(Cls)) == sizeof(Cls),
                "Incorrect struct padding used for automatic vertex attribute deduction. "
                "Make sure your struct isn't packed or use VE_GEN_ATTRIB_FN instead."
            );
            
            return result;
        }
        
        
        template <typename Vertex>
        inline void set_attrib_pointer(const vertex_attribute& attribute, std::size_t index) {
            if (attribute.type == GL_FLOAT) {
                glVertexAttribPointer(index, attribute.count, attribute.type, GL_FALSE, sizeof(Vertex), (void*) attribute.offset);
            } else if (attribute.type == GL_DOUBLE) {
                glVertexAttribLPointer(index, attribute.count, attribute.type, sizeof(Vertex), (void*) attribute.offset);
            } else {
                glVertexAttribIPointer(index, attribute.count, attribute.type, sizeof(Vertex), (void*) attribute.offset);
            }
        }
    }
    
    
    template <typename Vertex> inline void bind_vertex_layout(const context& ctx) {
        if constexpr (requires { Vertex::get_attributes(); }) {
            // Vertex has attribute list, use it.
            for (const vertex_attribute& attribute : Vertex::get_attributes()) {
                GLint index = glGetAttribLocation(ctx.current_program, attribute.name);
                if (index == -1) continue;
            
                glEnableVertexAttribArray(index);
                detail::set_attrib_pointer<Vertex>(attribute, index);
            }
        } else {
            // Vertex has no attribute list. Use Boost PFR to automatically extract attributes.
            const auto attributes = detail::make_anonymous_attribute_array<Vertex>();
            
            for (std::size_t i = 0; i < attributes.size(); ++i) {
                glEnableVertexAttribArray(i);
                detail::set_attrib_pointer<Vertex>(attributes[i], i);
            }
        }
    }
}