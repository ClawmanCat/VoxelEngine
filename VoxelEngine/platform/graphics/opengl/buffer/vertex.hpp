#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/buffer/attribute.hpp>


namespace ve::graphics {
    struct color_vertex_2d {
        vec2f position;
        vec4ub color;
        
        VE_GEN_ATTRIB_FN(color_vertex_2d, position, color);
    };
    
    struct color_vertex_3d {
        vec3f position;
        vec4ub color;
        
        VE_GEN_ATTRIB_FN(color_vertex_3d, position, color);
    };
    
    
    struct flat_texture_vertex_2d {
        vec2f position;
        vec2f uv;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(flat_texture_vertex_2d, position, uv, tex_index);
    };
    
    struct flat_texture_vertex_3d {
        vec3f position;
        vec2f uv;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(flat_texture_vertex_3d, position, uv, tex_index);
    };
    
    
    struct phong_texture_vertex_3d {
        vec3f position;
        vec2f uv;
        vec3f normal;
        u8 material;
        u8 tex_index;
        
        VE_GEN_ATTRIB_FN(phong_texture_vertex_3d, position, uv, normal, material, tex_index);
    };
}