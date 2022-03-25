#pragma once


// The vertices of a normalized screen-filling quad.
// Used when rendering with just a texture and no vertex buffer as source.
const vec2 quad_vertices[6] = {
    vec2(-1.0, -1.0), vec2(+1.0, -1.0), vec2(+1.0, +1.0),
    vec2(-1.0, -1.0), vec2(+1.0, +1.0), vec2(-1.0, +1.0)
};
