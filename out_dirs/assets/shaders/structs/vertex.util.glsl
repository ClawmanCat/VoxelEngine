#pragma once


#define PBR_VERTEX_BLOCK                \
PBRVertex {                             \
    vec3 position;                      \
    vec3 normal;                        \
    vec3 tangent;                       \
                                        \
    flat uint texture_index;            \
    vec2 uv_color;                      \
    vec2 uv_normal;                     \
    vec2 uv_material;                   \
}

#define INIT_PBR_VERTEX_BLOCK(dst)      \
dst.position = position;                \
dst.normal = normal;                    \
dst.tangent = tangent;                  \
                                        \
dst.texture_index = texture_index;      \
dst.uv_color = uv_color;                \
dst.uv_normal = uv_normal;              \
dst.uv_material = uv_material;




#define TEX_VERTEX_BLOCK                \
TextureVertex {                         \
    vec3 position;                      \
    vec2 uv;                            \
    flat uint texture_index;            \
}

#define INIT_TEX_VERTEX_BLOCK(dst)      \
dst.position = position;                \
dst.uv = uv;                            \
dst.texture_index = texture_index;




#define COLOR_VERTEX_BLOCK              \
ColorVertex {                           \
    vec3 position;                      \
    vec2 uv;                            \
    vec4 color;                         \
}

#define INIT_COLOR_VERTEX_BLOCK(dst)    \
dst.position = position;                \
dst.uv = uv;                            \
dst.color = color;




#define NO_VERTEX_BLOCK                 \
NoVertex {                              \
    vec2 uv;                            \
}

#define INIT_NO_VERTEX_BLOCK(dst)       \
dst.uv = uv;
