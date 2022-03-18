#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/thread/assert_main_thread.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>

#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>


namespace ve::gfx::opengl {
    enum class texture_filter : GLint {
        LINEAR, NEAREST
    };

    enum class texture_type : GLenum {
        TEXTURE_2D = GL_TEXTURE_2D, TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP
    };

    enum class texture_wrap : GLenum {
        REPEAT = GL_REPEAT, CLAMP = GL_CLAMP_TO_EDGE, REPEAT_MIRRORED = GL_MIRRORED_REPEAT, CLAMP_MIRRORED = GL_MIRROR_CLAMP_TO_EDGE
    };


    class texture {
    public:
        ve_shared_only(
            texture,
            const texture_format& fmt,
            const vec2ui& size,
            std::size_t mipmap_levels = get_context()->settings.num_mipmap_levels,
            texture_filter filter = texture_filter::NEAREST,
            texture_type type = texture_type::TEXTURE_2D,
            texture_wrap wrap = texture_wrap::REPEAT
        ) :
            type((GLenum) type),
            size(size),
            format(fmt),
            mipmap_levels(mipmap_levels),
            filter(filter),
            wrap(wrap)
        {
            assert_main_thread();

            glGenTextures(1, &id);
            VE_ASSERT(id, "Failed to create OpenGL texture.");

            glBindTexture(this->type, id);


            if (type == texture_type::TEXTURE_2D) {
                glTexStorage2D(this->type, (GLsizei) mipmap_levels, fmt.pixel_format, (GLsizei) size.x, (GLsizei) size.y);
            }

            else if (type == texture_type::TEXTURE_CUBE_MAP) {
                for (u32 side = 0; side < 6; ++side) {
                    glTexStorage2D(this->type + side, (GLsizei) mipmap_levels, fmt.pixel_format, (GLsizei) size.x, (GLsizei) size.y);
                }
            }


            glTexParameteri(this->type, GL_TEXTURE_WRAP_S, (GLenum) wrap);
            glTexParameteri(this->type, GL_TEXTURE_WRAP_T, (GLenum) wrap);

            glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, (filter == texture_filter::NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
            glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, (filter == texture_filter::NEAREST ? GL_NEAREST : GL_LINEAR));
            glGenerateMipmap(this->type);
        }


        ~texture(void) {
            if (id) glDeleteTextures(1, &id);
        }


        ve_immovable(texture);


        void set_parameter(GLenum name, GLint value) {
            assert_main_thread();

            glBindTexture(type, id);
            glTexParameteri(type, name, value);
        }


        void write(const image_rgba8& img, const vec2ui& where = vec2ui { 0 }) {
            assert_main_thread();

            VE_ASSERT(type == GL_TEXTURE_2D, "Currently, texture::write only supports 2D images.");
            VE_ASSERT(format == texture_format_RGBA8, "Currently, texture::write only supports RGBA8 images.");

            glBindTexture(type, id);

            glTexSubImage2D(
                type,
                0,
                (GLint) where.x, (GLint) where.y,
                (GLint) img.size.x, (GLint) img.size.y,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                img.data.data()
            );

            glGenerateMipmap(type);
        }


        image_rgba8 read(void) const {
            assert_main_thread();

            VE_ASSERT(type == GL_TEXTURE_2D, "Currently, texture::read only supports 2D images.");
            VE_ASSERT(format == texture_format_RGBA8, "Currently, texture::read only supports RGBA8 images.");

            image_rgba8 result;
            result.data.resize(size.x * size.y, RGBA8 { 0, 0, 0, 255 });
            result.size = size;

            glBindTexture(type, id);
            glGetTexImage(type, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.data.data());

            return result;
        }


        void bind(void) const {
            assert_main_thread();
            glBindTexture(type, id);
        }


        // Clears the texture to the given color.
        // Note: if the texture has n channels, only the first n values of 'value' are used.
        void clear(vec4f value) {
            bind();

            // TODO: Do this in a more robust way. Store scalar type in format data?
            GLenum channel_t      = format.name.back() == 'F' ? GL_FLOAT : GL_UNSIGNED_BYTE;
            vec4ub integer_value  = vec4ub(glm::round(value * 255.0f));
            const void* value_ptr = format.name.back() == 'F' ? (const void*) glm::value_ptr(value) : (const void*) glm::value_ptr(integer_value);

            glClearTexImage(id, 0, format.components, channel_t, value_ptr);
            glGenerateMipmap(this->type);
        }


        VE_GET_CREF(size);
        VE_GET_CREF(format);
        VE_GET_VAL(id);
        VE_GET_VAL(type);
        VE_GET_VAL(mipmap_levels);
        VE_GET_VAL(filter);
        VE_GET_VAL(wrap);
    private:
        GLuint id = 0;
        GLenum type;
        vec2ui size;

        texture_format format;
        std::size_t mipmap_levels;
        texture_filter filter;
        texture_wrap wrap;
    };
}