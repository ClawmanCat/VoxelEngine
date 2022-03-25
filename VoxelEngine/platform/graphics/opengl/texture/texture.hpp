#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/delayed_cast.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_base.hpp>


namespace ve::gfx::opengl {
    class texture : public texture_base {
    public:
        ve_shared_only(
            texture,
            const vec2ui& size,
            const texture_format& fmt = texture_format_RGBA8,
            std::size_t mipmap_levels = get_context()->settings.num_mipmap_levels,
            texture_filter filter     = texture_filter::NEAREST,
            texture_wrap wrap         = texture_wrap::REPEAT
        ) :
            texture_base(texture_type::TEXTURE_2D, size, fmt, mipmap_levels, filter, wrap)
        {
            glTexStorage2D((GLenum) get_gl_type(), (GLsizei) mipmap_levels, fmt.pixel_format, (GLsizei) size.x, (GLsizei) size.y);
            regenerate_mipmaps_bound();
        }


        template <typename Pixel> void write(const image<Pixel>& img, const vec2ui& where = vec2ui { 0 }) {
            bind();

            assert_pixel_matches_format<Pixel>(make_delayed_invoker<std::string>([&] {
                return cat("Cannot store image with pixel type ", ctti::nameof<Pixel>(), " to texture with format ", get_format().name, ".");
            }));


            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                (GLint) where.x,    (GLint) where.y,
                (GLint) img.size.x, (GLint) img.size.y,
                get_format().components,
                get_format().component_type,
                img.data.data()
            );

            regenerate_mipmaps();
        }


        template <typename Pixel = RGBA8> image<Pixel> read(void) const {
            bind();

            assert_pixel_matches_format<Pixel>(make_delayed_invoker<std::string>([&] {
                return cat("Cannot create image with pixel type ", ctti::nameof<Pixel>(), " from texture with format ", get_format().name, ".");
            }));


            auto result = filled_image(get_size(), Pixel { 0 });

            glGetTexImage(
                GL_TEXTURE_2D,
                0,
                get_format().components,
                get_format().component_type,
                result.data.data()
            );

            return result;
        }
    };
}