#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    struct framebuffer_attachment_template {
        enum attachment_type_t : GLenum { COLOR_BUFFER = GL_COLOR_ATTACHMENT0, DEPTH_BUFFER = GL_DEPTH_ATTACHMENT };


        // Attachment settings.
        std::string name;
        attachment_type_t attachment_type;
        // Note: if texture type has n channels, the first n values of the clear value are used.
        vec4f clear_value;
        std::function<bool(void)> should_clear = produce(true);

        // Underlying texture settings.
        const texture_format* tex_format;
        texture_type tex_type     = texture_type::TEXTURE_2D;
        texture_filter tex_filter = texture_filter::NEAREST;
        texture_wrap tex_wrap     = texture_wrap::CLAMP;


        explicit framebuffer_attachment_template(std::string name, attachment_type_t type = COLOR_BUFFER, const texture_format* fmt = nullptr)
            : name(std::move(name)), attachment_type(type)
        {
            if (!fmt) fmt = (type == COLOR_BUFFER)
                ? &get_context()->settings.color_buffer_format
                : &get_context()->settings.depth_buffer_format;

            tex_format  = fmt;
            clear_value = (type == COLOR_BUFFER) ? vec4f { 0, 0, 0, 0 } : vec4f { 1, 0, 0, 0 };
        }
    };
}