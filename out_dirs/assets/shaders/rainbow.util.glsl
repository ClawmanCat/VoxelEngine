float ve_impl_rainbow_boost_min(float x, float new_min) {
    return ((x * (1 - new_min)) + new_min);
}


vec3 rainbow(vec3 base, float u, float v, float t, float scale, float min_brightness) {
    float dr = ve_impl_rainbow_boost_min(sin(t + scale * (u * v)), min_brightness);
    float dg = ve_impl_rainbow_boost_min(cos(2 * t + scale * (u * v)), min_brightness);
    float db = ve_impl_rainbow_boost_min(-sin(3 * t + scale * (u * v)), min_brightness);

    return (base * (2.5f * vec3(dr, dg, db))) / 3.5f;
}


vec3 rainbow(vec3 base, float u, float v, float t, float scale) {
    return rainbow(base, u, v, t, scale, 0.75f);
}


vec3 rainbow(vec3 base, float u, float v, float t) {
    return rainbow(base, u, v, t, 100.0f, 0.75f);
}