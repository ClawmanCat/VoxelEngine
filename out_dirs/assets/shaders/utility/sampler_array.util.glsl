#pragma once

#include "boost/preprocessor.hpp"


#ifndef SAMPLER_ARRAY_SIZE
    #define SAMPLER_ARRAY_SIZE BOOST_PP_MIN(VE_MAX_FS_SAMPLERS, 32)
#endif


#define VE_IMPL_SAMPLE_ARRAY_MACRO(Z, N, D) \
case N: return texture(samplers[N], uv);


// It is undefined behaviour to pick a sampler from a sampler array using a non-constant expression and then sample from it.
// To overcome this limitation, simply unroll a loop to check the index against a constant and then conditionally sample.
#define VE_ARRAY_SAMPLER_FN(Name, SamplerT, SampledT, AccessT, Size)                                \
SampledT Name##_or(SamplerT samplers[Size], uint index, AccessT uv, SampledT default_value) {       \
    switch (index) {                                                                                \
        BOOST_PP_REPEAT(                                                                            \
            Size,                                                                                   \
            VE_IMPL_SAMPLE_ARRAY_MACRO,                                                             \
            _                                                                                       \
        )                                                                                           \
    }                                                                                               \
                                                                                                    \
    return default_value;                                                                           \
}                                                                                                   \
                                                                                                    \
SampledT Name(SamplerT samplers[Size], uint index, AccessT uv) {                                    \
    return Name##_or(samplers, index, uv, SampledT(0));                                             \
}


VE_ARRAY_SAMPLER_FN(sample_array, sampler2D, vec4, vec2, SAMPLER_ARRAY_SIZE)

