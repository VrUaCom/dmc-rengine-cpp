#ifndef RENGINE_LSG_SKIN_MATERIAL_GLSL
#define RENGINE_LSG_SKIN_MATERIAL_GLSL

#include "../include/rengine/lsg/skin_contract.inc"

layout(set = 0, binding = 3, std140) uniform SkinMaterial {
    vec4 pigments;   // melanin, haemoglobin, carotene, age profile
    vec4 surface;    // oiliness, hydration, roughness bias, coat strength
    vec4 pores;      // density, scale, depth, follicle density
    vec4 features;   // freckle density, meso strength, micro strength, wrinkle bias
    vec4 physiology; // perfusion, sweat, temperature normalized, subsurface strength
} skin;

vec3 lsg_skin_base_colour() {
    float melanin_mix = pow(clamp(skin.pigments.x, 0.0, 1.0), RENGINE_SKIN_MELANIN_EXPONENT) * RENGINE_SKIN_MELANIN_MIX_SCALE;
    vec3 colour = mix(vec3(RENGINE_SKIN_LIGHT_R, RENGINE_SKIN_LIGHT_G, RENGINE_SKIN_LIGHT_B),
                      vec3(RENGINE_SKIN_DARK_R, RENGINE_SKIN_DARK_G, RENGINE_SKIN_DARK_B), melanin_mix);

    float blood = skin.pigments.y - RENGINE_SKIN_HAEM_BASELINE;
    colour += vec3(RENGINE_SKIN_BLOOD_R, RENGINE_SKIN_BLOOD_G, RENGINE_SKIN_BLOOD_B) * blood;

    float carotene = skin.pigments.z - RENGINE_SKIN_CAROTENE_BASELINE;
    colour += vec3(RENGINE_SKIN_CAROTENE_R, RENGINE_SKIN_CAROTENE_G, RENGINE_SKIN_CAROTENE_B) * carotene;

    float temperature = skin.physiology.z - RENGINE_SKIN_TEMP_BASELINE;
    colour += vec3(RENGINE_SKIN_TEMP_R, RENGINE_SKIN_TEMP_G, RENGINE_SKIN_TEMP_B) * temperature;
    return clamp(colour, vec3(RENGINE_SKIN_CLAMP_MIN_R, RENGINE_SKIN_CLAMP_MIN_G, RENGINE_SKIN_CLAMP_MIN_B), vec3(RENGINE_SKIN_CLAMP_MAX_R, RENGINE_SKIN_CLAMP_MAX_G, RENGINE_SKIN_CLAMP_MAX_B));
}

float lsg_skin_base_roughness() {
    return clamp(RENGINE_SKIN_ROUGH_BASE + (skin.surface.z - 0.5) * RENGINE_SKIN_ROUGH_BIAS_GAIN
                     - skin.surface.x * RENGINE_SKIN_ROUGH_OIL_GAIN
                     - skin.surface.y * RENGINE_SKIN_ROUGH_HYDRATION_GAIN
                     - skin.physiology.y * RENGINE_SKIN_ROUGH_SWEAT_GAIN
                     + skin.pigments.w * RENGINE_SKIN_ROUGH_AGE_GAIN,
                 RENGINE_SKIN_ROUGH_MIN, RENGINE_SKIN_ROUGH_MAX);
}

float lsg_skin_specular_scale() {
    return RENGINE_SKIN_SPEC_BASE + clamp(skin.surface.w, 0.0, 1.0) * RENGINE_SKIN_SPEC_COAT_GAIN;
}

#endif
