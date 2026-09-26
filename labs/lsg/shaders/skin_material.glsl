#ifndef RENGINE_LSG_SKIN_MATERIAL_GLSL
#define RENGINE_LSG_SKIN_MATERIAL_GLSL

#include "../include/rengine/lsg/skin_contract.inc"

#include "character_gpu_state.glsl"

vec3 lsg_skin_base_colour() {
    float melanin_mix = pow(clamp(character_state.pigments.x, 0.0, 1.0), RENGINE_SKIN_MELANIN_EXPONENT) * RENGINE_SKIN_MELANIN_MIX_SCALE;
    vec3 colour = mix(vec3(RENGINE_SKIN_LIGHT_R, RENGINE_SKIN_LIGHT_G, RENGINE_SKIN_LIGHT_B),
                      vec3(RENGINE_SKIN_DARK_R, RENGINE_SKIN_DARK_G, RENGINE_SKIN_DARK_B), melanin_mix);

    float blood = character_state.pigments.y - RENGINE_SKIN_HAEM_BASELINE;
    colour += vec3(RENGINE_SKIN_BLOOD_R, RENGINE_SKIN_BLOOD_G, RENGINE_SKIN_BLOOD_B) * blood;

    float carotene = character_state.pigments.z - RENGINE_SKIN_CAROTENE_BASELINE;
    colour += vec3(RENGINE_SKIN_CAROTENE_R, RENGINE_SKIN_CAROTENE_G, RENGINE_SKIN_CAROTENE_B) * carotene;

    float temperature = character_state.physiology.z - RENGINE_SKIN_TEMP_BASELINE;
    colour += vec3(RENGINE_SKIN_TEMP_R, RENGINE_SKIN_TEMP_G, RENGINE_SKIN_TEMP_B) * temperature;
    return clamp(colour, vec3(RENGINE_SKIN_CLAMP_MIN_R, RENGINE_SKIN_CLAMP_MIN_G, RENGINE_SKIN_CLAMP_MIN_B), vec3(RENGINE_SKIN_CLAMP_MAX_R, RENGINE_SKIN_CLAMP_MAX_G, RENGINE_SKIN_CLAMP_MAX_B));
}

float lsg_skin_base_roughness() {
    return clamp(RENGINE_SKIN_ROUGH_BASE + (character_state.surface.z - 0.5) * RENGINE_SKIN_ROUGH_BIAS_GAIN
                     - character_state.surface.x * RENGINE_SKIN_ROUGH_OIL_GAIN
                     - character_state.surface.y * RENGINE_SKIN_ROUGH_HYDRATION_GAIN
                     - character_state.physiology.y * RENGINE_SKIN_ROUGH_SWEAT_GAIN
                     + character_state.pigments.w * RENGINE_SKIN_ROUGH_AGE_GAIN,
                 RENGINE_SKIN_ROUGH_MIN, RENGINE_SKIN_ROUGH_MAX);
}

float lsg_skin_specular_scale() {
    return RENGINE_SKIN_SPEC_BASE + clamp(character_state.surface.w, 0.0, 1.0) * RENGINE_SKIN_SPEC_COAT_GAIN;
}

#endif
