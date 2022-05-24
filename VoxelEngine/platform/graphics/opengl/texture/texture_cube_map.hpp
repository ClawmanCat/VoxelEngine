#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/direction.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture_base.hpp>

#include <gl/glew.h>
#include <magic_enum.hpp>


namespace ve::gfx::opengl {
    enum class cube_map_face : GLenum {
        BACK   = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        FRONT  = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        TOP    = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        BOTTOM = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        RIGHT  = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        LEFT   = GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    };


    constexpr direction_t direction_for_cube_map_face(cube_map_face face) {
        switch (face) {
            case cube_map_face::BACK:   return direction_from_vector(direction::BACKWARD);
            case cube_map_face::FRONT:  return direction_from_vector(direction::FORWARD);
            case cube_map_face::TOP:    return direction_from_vector(direction::UP);
            case cube_map_face::BOTTOM: return direction_from_vector(direction::DOWN);
            case cube_map_face::RIGHT:  return direction_from_vector(direction::RIGHT);
            case cube_map_face::LEFT:   return direction_from_vector(direction::LEFT);
        }
    }


    constexpr cube_map_face cube_map_face_for_direction(direction_t direction) {
        const auto direction_vector = directions[direction];

        if (direction_vector == direction::BACKWARD) return cube_map_face::BACK;
        if (direction_vector == direction::FORWARD)  return cube_map_face::FRONT;
        if (direction_vector == direction::UP)       return cube_map_face::TOP;
        if (direction_vector == direction::DOWN)     return cube_map_face::BOTTOM;
        if (direction_vector == direction::RIGHT)    return cube_map_face::RIGHT;
        if (direction_vector == direction::LEFT)     return cube_map_face::LEFT;

        VE_UNREACHABLE;
    }


    class texture_cube_map : public texture_base {
    public:
        ve_shared_only(
            texture_cube_map,
            const vec2ui& size,
            const texture_format& fmt = texture_format_RGBA8,
            std::size_t mipmap_levels = get_context()->settings.num_mipmap_levels,
            texture_filter filter     = texture_filter::NEAREST,
            texture_wrap wrap         = texture_wrap::REPEAT
        ) :
            texture_base(texture_type::TEXTURE_CUBE_MAP, size, fmt, mipmap_levels, filter, wrap)
        {
            VE_DEBUG_ASSERT(size.x == size.y, "Cube map textures must be square (Provided size is ", size, ").");

            glTexStorage2D((GLenum) get_gl_type(), (GLsizei) mipmap_levels, fmt.pixel_format, (GLsizei) size.x, (GLsizei) size.y);
            regenerate_mipmaps_bound();
        }


        template <typename Pixel> void write(cube_map_face face, const image<Pixel>& img, const vec2ui& where = vec2ui { 0 }) {
            bind();

            assert_pixel_matches_format<Pixel>(make_delayed_invoker<std::string>([&] {
                return cat("Cannot store image with pixel type ", ctti::nameof<Pixel>(), " to texture with format ", get_format().name, ".");
            }));


            glTexSubImage2D(
                (GLenum) face,
                0,
                (GLint) where.x,    (GLint) where.y,
                (GLint) img.size.x, (GLint) img.size.y,
                get_format().components,
                get_format().component_type,
                img.data.data()
            );

            regenerate_mipmaps();
        }


        template <typename Pixel = RGBA8> image<Pixel> read(cube_map_face face) const {
            bind();

            assert_pixel_matches_format<Pixel>(make_delayed_invoker<std::string>([&] {
                return cat("Cannot create image with pixel type ", ctti::nameof<Pixel>(), " from texture with format ", get_format().name, ".");
            }));


            auto result = filled_image(get_size(), Pixel { 0 });

            glGetTexImage(
                (GLenum) face,
                0,
                get_format().components,
                get_format().component_type,
                result.data.data()
            );

            return result;
        }
    };
}