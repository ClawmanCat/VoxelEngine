#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/texture.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>
#include <VoxelEngine/platform/graphics/opengl/common.hpp>

#include <GL/glew.h>

#include <array>


namespace ve::graphics {
    struct texture_parameters {
        enum class filter_type { NEAREST, LINEAR };
        
        // Use 1 for no mipmapping.
        u8 mipmap_levels          = 4;
        GLenum texture_wrap       = GL_REPEAT;
        filter_type min_filter    = filter_type::NEAREST;
        filter_type mag_filter    = filter_type::NEAREST;
        // Ignored if mipmapping is disabled.
        filter_type mipmap_filter = filter_type::NEAREST;
    };
    
    
    namespace detail {
        enum class filter_direction { MIN_FILTER = 0, MAG_FILTER = 1 };
        
        
        inline GLint get_gl_filter_enum(const texture_parameters& params, filter_direction direction) {
            using filter_type = texture_parameters::filter_type;
            
            constexpr std::array filter_fields {
                &texture_parameters::min_filter,
                &texture_parameters::mag_filter
            };
            
            if (params.mipmap_levels == 1 || direction == filter_direction::MAG_FILTER) {
                // No mipmapping.
                return (params.*(filter_fields[(u32) direction]) == filter_type::NEAREST)
                    ? GL_NEAREST
                    : GL_LINEAR;
            }
            
            if (params.mipmap_filter == filter_type::NEAREST) {
                return params.min_filter == filter_type::NEAREST
                    ? GL_NEAREST_MIPMAP_NEAREST
                    : GL_LINEAR_MIPMAP_NEAREST;
            } else {
                return params.min_filter == filter_type::NEAREST
                    ? GL_NEAREST_MIPMAP_LINEAR
                    : GL_LINEAR_MIPMAP_LINEAR;
            }
        }
        
        
        inline texture initialize_texture(const vec2ui& size, texture_format fmt = texture_format::RGBA8, const texture_parameters& params = {}) {
            using filter_dir = detail::filter_direction;
    
            GLuint id;
            glGenTextures(1, &id);
            VE_ASSERT(id, "Failed to create texture.");
    
            texture result { id, size, params.mipmap_levels, fmt };
            
            glBindTexture(GL_TEXTURE_2D, id);
            glTexStorage2D(
                GL_TEXTURE_2D,
                params.mipmap_levels,
                texture_format_info[(u32) fmt].storage_format,
                size.x,
                size.y
            );
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, detail::get_gl_filter_enum(params, filter_dir::MIN_FILTER));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, detail::get_gl_filter_enum(params, filter_dir::MAG_FILTER));
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.texture_wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.texture_wrap);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            return result;
        }
        
        
        inline void store_texture_data(texture& tex, const io::image& img, const vec2ui& where = { 0, 0 }) {
            VE_ASSERT(tex.get_id(), "Cannot store to empty texture.");
            VE_ASSERT(glm::all(where + img.size <= tex.get_size()), "Cannot insert texture data outside texture bounds.");
    
            glBindTexture(GL_TEXTURE_2D, tex.get_id());
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                where.x,
                where.y,
                img.size.x,
                img.size.y,
                texture_format_info[(u32) tex.get_format()].channel_format,
                GL_UNSIGNED_BYTE, // Pixel format of the data, not the GPU pixel format.
                &img.pixels[0]
            );
            
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
}