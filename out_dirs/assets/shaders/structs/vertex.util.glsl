#pragma once

#include "boost/preprocessor.hpp"


// For every vertex type, a sequence of its attributes as a tuple of (interpolation qualifier, type, name).
#define PBR_VERTEX_ATTRIBS                                                          \
((smooth, vec3, position))((smooth, vec3, normal))((smooth, vec3, tangent))         \
((flat, uint, texture_index))                                                       \
((smooth, vec2, uv_color))((smooth, vec2, uv_normal))((smooth, vec2, uv_material))

#define TEX_VERTEX_ATTRIBS                                                          \
((smooth, vec3, position))((smooth, vec2, uv))((flat, uint, texture_index))

#define COLOR_VERTEX_ATTRIBS                                                        \
((smooth, vec3, position))((smooth, vec2, uv))((smooth, vec4, color))

#define NO_VERTEX_ATTRIBS                                                           \
((smooth, vec2, uv))


// Create an interface block for the given vertex type.
#define VE_IMPL_VB_MACRO(R, D, E) \
BOOST_PP_TUPLE_ELEM(0, E) BOOST_PP_TUPLE_ELEM(1, E) BOOST_PP_TUPLE_ELEM(2, E);

#define VERTEX_BLOCK(Name, Attribs)         \
Name {                                      \
    BOOST_PP_SEQ_FOR_EACH(                  \
        VE_IMPL_VB_MACRO,                   \
        _,                                  \
        Attribs                             \
    )                                       \
}

#define PBR_VERTEX_BLOCK   VERTEX_BLOCK(PBRVertex,     PBR_VERTEX_ATTRIBS)
#define TEX_VERTEX_BLOCK   VERTEX_BLOCK(TextureVertex, TEX_VERTEX_ATTRIBS)
#define COLOR_VERTEX_BLOCK VERTEX_BLOCK(ColorVertex,   COLOR_VERTEX_ATTRIBS)
#define NO_VERTEX_BLOCK    VERTEX_BLOCK(NoVertex,      NO_VERTEX_ATTRIBS)


// Create vertex input definitions for the given vertex type.
#define VE_IMPL_VL_MACRO(R, D, E) \
in BOOST_PP_TUPLE_ELEM(1, E) BOOST_PP_TUPLE_ELEM(2, E);

#define VERTEX_LAYOUT(Attribs)              \
BOOST_PP_SEQ_FOR_EACH(                      \
    VE_IMPL_VL_MACRO,                       \
    _,                                      \
    Attribs                                 \
)

#define PBR_VERTEX_LAYOUT   VERTEX_LAYOUT(PBR_VERTEX_ATTRIBS)
#define TEX_VERTEX_LAYOUT   VERTEX_LAYOUT(TEX_VERTEX_ATTRIBS)
#define COLOR_VERTEX_LAYOUT VERTEX_LAYOUT(COLOR_VERTEX_ATTRIBS)
#define NO_VERTEX_LAYOUT    VERTEX_LAYOUT(NO_VERTEX_ATTRIBS)


// Initialize the interface block 'dst' using vertex attributes generated with VERTEX_INPUTS
#define VE_IMPL_INIT_MACRO(R, D, E) \
D.BOOST_PP_TUPLE_ELEM(2, E) = BOOST_PP_TUPLE_ELEM(2, E);

#define BLOCK_INITIALIZER(Dst, Attribs)     \
BOOST_PP_SEQ_FOR_EACH(                      \
    VE_IMPL_INIT_MACRO,                     \
    Dst,                                    \
    Attribs                                 \
)

#define INIT_PBR_VERTEX_BLOCK(Dst)   BLOCK_INITIALIZER(Dst, PBR_VERTEX_ATTRIBS)
#define INIT_TEX_VERTEX_BLOCK(Dst)   BLOCK_INITIALIZER(Dst, TEX_VERTEX_ATTRIBS)
#define INIT_COLOR_VERTEX_BLOCK(Dst) BLOCK_INITIALIZER(Dst, COLOR_VERTEX_ATTRIBS)
#define INIT_NO_VERTEX_BLOCK(Dst)    BLOCK_INITIALIZER(Dst, NO_VERTEX_ATTRIBS)
