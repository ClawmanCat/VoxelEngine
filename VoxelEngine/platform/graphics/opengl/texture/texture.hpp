#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/texture/format.hpp>

#include <GL/glew.h>


namespace ve::graphics {
    class texture {
    public:
        texture(GLuint id, const vec2ui& size, u8 mipmap_levels, texture_format fmt = texture_format::RGBA8) :
            id(id),
            size(size),
            format(fmt),
            mipmap_levels(mipmap_levels)
        {}
        
        ~texture(void) {
            if (id) glDeleteTextures(1, &id);
        }
        
        ve_swap_move_only(texture, id, size, format, mipmap_levels);
        
    
        ve_eq_comparable_fields(texture, id);
        
        VE_GET_VAL(id);
        VE_GET_VAL(mipmap_levels);
        VE_GET_VAL(format);
        VE_GET_VAL(size);
        
    private:
        GLuint id = 0;
        vec2ui size;
        texture_format format = texture_format::RGBA8;
        u8 mipmap_levels;
    };
}