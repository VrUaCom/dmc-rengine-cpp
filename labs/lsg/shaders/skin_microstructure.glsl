#ifndef RENGINE_LSG_SKIN_MICROSTRUCTURE_GLSL
#define RENGINE_LSG_SKIN_MICROSTRUCTURE_GLSL

#include "../include/rengine/lsg/skin_contract.inc"

float lsg_skin_freckle_mask(vec3 p, uint seed) {
    float n = value_noise(p / 0.0045, seed ^ 0xF1357AEAu);
    float threshold = mix(RENGINE_SKIN_FRECKLE_THRESHOLD_SPARSE, RENGINE_SKIN_FRECKLE_THRESHOLD_DENSE, clamp(skin.features.x, 0.0, 1.0));
    return smoothstep(threshold, 1.0, n);
}

float lsg_skin_follicle_influence(vec3 p, uint seed) {
    float n = value_noise(p / 0.0012, seed ^ 0x7F4A7C15u);
    float threshold = mix(RENGINE_SKIN_FOLLICLE_THRESHOLD_SPARSE, RENGINE_SKIN_FOLLICLE_THRESHOLD_DENSE, clamp(skin.pores.w, 0.0, 1.0));
    return smoothstep(threshold, 1.0, n);
}

float lsg_skin_wrinkle_height(vec3 p, uint seed, float footprint_m) {
    float directional = value_noise(
        vec3(p.x / 0.0018, p.y / 0.00072, p.z / 0.0018),
        seed ^ 0x91E10DA5u) - 0.5;
    float filter_weight = clamp((RENGINE_SKIN_WRINKLE_FILTER_M - footprint_m) / RENGINE_SKIN_WRINKLE_FILTER_M, 0.0, 1.0);
    return directional * clamp(skin.features.z, 0.0, 1.0) *
           clamp(skin.features.w, 0.0, 1.0) * RENGINE_SKIN_WRINKLE_AMPLITUDE_M * filter_weight;
}

#endif
