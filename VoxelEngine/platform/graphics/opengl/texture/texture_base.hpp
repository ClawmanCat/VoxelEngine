#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/thread/assert_main_thread.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/api_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>

#include <gl/glew.h>


namespace ve::gfx::opengl {
    enum class texture_filter {
        LINEAR, NEAREST
    };

    namespace detail {
        inline GLenum min_filter_for_tex_filter(texture_filter filter) {
            return filter == texture_filter::NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
        }

        inline GLenum mag_filter_for_tex_filter(texture_filter filter) {
            return filter == texture_filter::NEAREST ? GL_NEAREST : GL_LINEAR;
        }
    }


    enum class texture_type : GLenum {
        TEXTURE_2D = GL_TEXTURE_2D, TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP
    };


    enum class texture_wrap : GLenum {
        REPEAT          = GL_REPEAT,
        REPEAT_MIRRORED = GL_MIRRORED_REPEAT,
        CLAMP           = GL_CLAMP_TO_EDGE,
        CLAMP_MIRRORED  = GL_MIRROR_CLAMP_TO_EDGE
    };



    // Base class for other texture types. This class cannot be instantiated directly.
    template <typename Derived> class texture_base : public std::enable_shared_from_this<Derived> {
    protected:
        ve_shared_only(
            texture_base,
            texture_type type,
            const vec2ui& size,
            const texture_format& fmt,
            std::size_t mipmap_levels = get_context()->settings.num_mipmap_levels,
            texture_filter filter = texture_filter::NEAREST,
            texture_wrap wrap = texture_wrap::REPEAT
        ) :
            size(size),
            mipmap_levels(mipmap_levels),
            type(type),
            filter(filter),
            wrap(wrap),
            format(fmt)
        {
            assert_main_thread();

            glGenTextures(1, &id);
            VE_ASSERT(id, "Failed to create ", ctti::nameof<Derived>() ," texture.");
            bind();

            set_parameter_bound(GL_TEXTURE_WRAP_S, (GLenum) wrap);
            set_parameter_bound(GL_TEXTURE_WRAP_T, (GLenum) wrap);
            set_parameter_bound(GL_TEXTURE_MIN_FILTER, detail::min_filter_for_tex_filter(filter));
            set_parameter_bound(GL_TEXTURE_MAG_FILTER, detail::mag_filter_for_tex_filter(filter));
        }

    public:
        ve_immovable(texture_base);


        virtual ~texture_base(void) {
            if (id) glDeleteTextures(1, &id);
        }


        void set_parameter(GLenum name, std::integral auto value) {
            bind();
            set_parameter_bound(name, value);
        }


        void set_parameter(GLenum name, std::floating_point auto value) {
            bind();
            set_parameter_bound(name, value);
        }


        void regenerate_mipmaps(void) {
            bind();
            regenerate_mipmaps_bound();
        }


        virtual void bind(void) const {
            assert_main_thread();
            glBindTexture((GLenum) type, id);
        }


        GLenum get_gl_type(void) const { return (GLenum) type; }

        VE_GET_VAL(id);
        VE_GET_VAL(size);
        VE_GET_VAL(mipmap_levels);
        VE_GET_VAL(type);
        VE_GET_VAL(filter);
        VE_GET_VAL(wrap);
        VE_GET_CREF(format);
    private:
        GLuint id = 0;
        vec2ui size;
        std::size_t mipmap_levels;

        texture_type type;
        texture_filter filter;
        texture_wrap wrap;
        texture_format format;

    protected:
        // Equivalent to public methods, except the texture is already assumed to be bound.
        void set_parameter_bound(GLenum name, std::integral auto value) {
            glTexParameteri((GLenum) type, name, (GLint) value);
        }

        void set_parameter_bound(GLenum name, std::floating_point auto value) {
            glTexParameterf((GLenum) type, name, (GLfloat) value);
        }

        void regenerate_mipmaps_bound(void) {
            glGenerateMipmap((GLenum) type);
        }
    };
}