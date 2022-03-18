#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>

#include <magic_enum.hpp>
#include <gl/glew.h>


namespace ve::gfx::opengl::detail {
    GLenum get_gl_attribute_type(typename reflect::object_type::base_type_t type, std::size_t size);


    struct vertex_layout {
        void bind(void) const {
            for (const auto& attribute : attributes) {
                glVertexAttribPointer(
                    attribute.location,
                    attribute.vector_rows,
                    attribute.base_type,
                    GL_FALSE,
                    vertex_stride,
                    attribute.offset_ptr
                );

                glEnableVertexAttribArray(attribute.location);
            }
        }


        struct attribute_data {
            GLuint location;
            GLuint vector_rows;
            GLenum base_type;
            const void* offset_ptr;
        };

        std::vector<attribute_data> attributes;
        GLsizei vertex_stride;
    };


    template <typename Vertex>
    inline vertex_layout generate_vertex_layout(const reflect::stage& input_stage) {
        auto pairs = combine_into_pairs(
            Vertex::get_vertex_layout(),
            input_stage.inputs,
            [](const vertex_attribute& attrib, const reflect::attribute& input) {
                return attrib.name == input.name;
            }
        );


        vertex_layout result { .vertex_stride = sizeof(Vertex) };
        result.attributes.reserve(pairs.matched.size());

        for (const auto& [attrib, input] : pairs.matched) {
            result.attributes.push_back(vertex_layout::attribute_data {
                .location    = (GLuint) input->location,
                .vector_rows = (GLuint) attrib->type.rows,
                .base_type   = get_gl_attribute_type(attrib->type.base_type, attrib->type.base_size),
                .offset_ptr  = (const void*) attrib->offset
            });
        }

        return result;
    }


    inline GLenum get_gl_attribute_type(typename reflect::object_type::base_type_t type, std::size_t size) {
        using BT = reflect::object_type::base_type_t;

        #define ve_impl_unknown_attrib_size \
        VE_ASSERT(false, "No known conversion from ", magic_enum::enum_name(type), " of size ", size, " to OpenGL type.")


        if (type == BT::FLOAT) {
            switch (size) {
                case 4: return GL_FLOAT;
                case 8: return GL_DOUBLE;
                default: ve_impl_unknown_attrib_size;
            }
        }

        else if (type == BT::INT) {
            switch (size) {
                case 1: return GL_BYTE;
                case 2: return GL_SHORT;
                case 4: return GL_INT;
                default: ve_impl_unknown_attrib_size;
            }
        }

        else if (type == BT::UINT) {
            switch(size) {
                case 1: return GL_UNSIGNED_BYTE;
                case 2: return GL_UNSIGNED_SHORT;
                case 4: return GL_UNSIGNED_INT;
                default: ve_impl_unknown_attrib_size;
            }
        }

        else {
            VE_ASSERT(false, "Unsupported vertex base type. Vertices may only contain scalar and vector elements.");
        }

        VE_UNREACHABLE;
    }
}