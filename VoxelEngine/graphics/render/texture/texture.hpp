#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/comparable.hpp>
#include <VoxelEngine/utils/hashable.hpp>

#include <GL/glew.h>

#include <cstddef>


namespace ve {
    struct texture {
        GLuint id;
        
        ve_make_comparable(texture);
    };
    
    
    struct subtexture {
        texture tex;
        vec4f uv;
        
        ve_make_eq_comparable(subtexture);
    };
}

ve_make_hashable(ve::texture, id);
ve_make_hashable(ve::subtexture, tex, uv);