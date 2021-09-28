#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    enum class texture_filter : GLint { LINEAR, NEAREST };


    class texture {
    public:
        ve_shared_only(
            texture,
            const texture_format& fmt,
            const vec2ui& size,
            std::size_t mipmap_levels = get_context()->settings.num_mipmap_levels,
            texture_filter filter = texture_filter::NEAREST
        ) :
            uv { vec2f { 0.0f }, vec2f { 1.0f } },
            size(size),
            format(fmt),
            filter(filter)
        {
            glGenTextures(1, &id);
            VE_ASSERT(id, "Failed to create OpenGL texture.");

            glBindTexture(GL_TEXTURE_2D, id);
            glTexStorage2D(GL_TEXTURE_2D, (GLsizei) mipmap_levels, fmt.pixel_format, (GLsizei) size.x, (GLsizei) size.y);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter == texture_filter::NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filter == texture_filter::NEAREST ? GL_NEAREST : GL_LINEAR));
            glGenerateMipmap(GL_TEXTURE_2D);
        }


        ~texture(void) {
            if (id) glDeleteTextures(1, &id);
        }


        ve_immovable(texture);


        void write(const image_rgba8& img, const vec2ui& where = vec2ui { 0 }) {
            VE_ASSERT(format.components == GL_RGBA, "Cannot write RGBA image to non-RGBA texture.");


            glBindTexture(GL_TEXTURE_2D, id);

            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                (GLint) where.x, (GLint) where.y,
                (GLint) img.size.x, (GLint) img.size.y,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                img.data.data()
            );

            glGenerateMipmap(GL_TEXTURE_2D);
        }


        VE_GET_CREF(uv);
        VE_GET_CREF(size);
        VE_GET_CREF(format);
        VE_GET_VAL(id);
        VE_GET_VAL(mipmap_levels);
        VE_GET_VAL(filter);
    private:
        GLuint id = 0;

        std::array<vec2f, 2> uv;
        vec2ui size;

        texture_format format;
        std::size_t mipmap_levels;
        texture_filter filter;
    };
}