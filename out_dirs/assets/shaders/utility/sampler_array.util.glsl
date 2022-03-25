#pragma once

#include "boost/preprocessor.hpp"


#ifndef SAMPLER_ARRAY_SIZE
    #define SAMPLER_ARRAY_SIZE VE_MAX_FS_SAMPLERS
#endif


#define VE_IMPL_SAMPLE_ARRAY_MACRO(Z, N, D) \
case N: return texture(samplers[N], uv);


// It is undefined behaviour to pick a sampler from a sampler array using a non-constant expression and then sample from it.
// To overcome this limitation, simply unroll a loop to check the index against a constant and then conditionally sample.
vec4 sample_array(sampler2D samplers[SAMPLER_ARRAY_SIZE], uint index, vec2 uv) {
    switch (index) {
        BOOST_PP_REPEAT(
            SAMPLER_ARRAY_SIZE,
            VE_IMPL_SAMPLE_ARRAY_MACRO,
            _
        );
    }

    // Fallback option, just return 0.
    return vec4(0);
}
