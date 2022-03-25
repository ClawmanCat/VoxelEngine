#pragma once


#define GAUSSIAN_HORIZONTAL 0
#define GAUSSIAN_VERTICAL   1

#ifndef GAUSSIAN_SIZE_LIMIT
    #define GAUSSIAN_SIZE_LIMIT 16
#endif


struct GaussianData {
    float weights[GAUSSIAN_SIZE_LIMIT];
    uint populated_weights;
    float range;
};


struct BloomData {
    vec3 luma_conversion_weights;
    float bloom_intensity;
    float bloom_threshold;
};
