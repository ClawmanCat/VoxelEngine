#pragma once


// Returns f is f is nonzero, or epsilon otherwise. Used to prevent division by zero.
float unzero(float f) {
    return float(f == 0) * epsilon + f;
}


bool has_nan(vec2 pixel) { return any(isnan(pixel)); }
bool has_nan(vec3 pixel) { return any(isnan(pixel)); }
bool has_nan(vec4 pixel) { return any(isnan(pixel)); }
