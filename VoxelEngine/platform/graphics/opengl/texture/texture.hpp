#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/delayed_cast.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_base.hpp>

#include <glm/gtc/type_ptr.hpp>


namespace ve::gfx::opengl {
    class texture : public texture_base<texture> {
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


        template <typename Pixel> void clear(const Pixel& value) {
            bind();

            assert_pixel_matches_format<Pixel>(make_delayed_invoker<std::string>([&] {
                return cat("Cannot use pixel of type ", ctti::nameof<Pixel>(), " to clear texture with format ", get_format().name, ".");
            }));

            glClearTexImage(get_id(), 0, get_format().components, get_format().component_type, glm::value_ptr(value));
            regenerate_mipmaps();
        }


        // Equivalent to above, but instead of exactly matching the pixel type, the method just uses the first n channels of value,
        // where n is the number of channels of the pixel format, and converts the channels to the appropriate format.
        void clear_simple(const vec4f& value) {
            bind();

            vec4ub integer_value  = vec4ub { glm::round(value * 255.0f) };
            void*  value_ptr      = get_format().is_integer_texture() ? (void*) glm::value_ptr(integer_value) : (void*) glm::value_ptr(value);
            GLenum component_type = get_format().is_integer_texture() ? GL_UNSIGNED_BYTE : GL_FLOAT;

            glClearTexImage(get_id(), 0, get_format().components, component_type, value_ptr);
            regenerate_mipmaps();
        }

    private:
        template <typename Pixel> void assert_pixel_matches_format(auto error_message) const {
            using PixTraits = meta::glm_traits<Pixel>;
            using VT = typename PixTraits::value_type;


            const auto& format = get_format();
            
            VE_DEBUG_ASSERT(
                (std::is_floating_point_v<VT> ? format.is_floating_point_texture() : format.is_integer_texture()) &&
                ranges::all_of(format.channel_depths | views::take(format.num_channels), [] (u8 depth) { return depth == 8 * sizeof(VT); }) &&
                format.num_channels == PixTraits::num_rows,
                error_message
            );
        }
    };
}