#ifndef RENGINE_LSG_SKIN_MATERIAL_GLSL
#define RENGINE_LSG_SKIN_MATERIAL_GLSL

layout(set = 0, binding = 3, std140) uniform SkinMaterial {
    vec4 pigments;   // melanin, haemoglobin, carotene, age profile
    vec4 surface;    // oiliness, hydration, roughness bias, coat strength
    vec4 pores;      // density, scale, depth, follicle density
    vec4 features;   // freckle density, meso strength, micro strength, wrinkle bias
    vec4 physiology; // perfusion, sweat, temperature normalized, subsurface strength
} skin;

vec3 lsg_skin_base_colour() {
    float melanin_mix = pow(clamp(skin.pigments.x, 0.0, 1.0), 0.82) * 0.90;
    vec3 colour = mix(vec3(0.66, 0.39, 0.29),
                      vec3(0.12, 0.050, 0.030), melanin_mix);

    float blood = skin.pigments.y - 0.45;
    colour += vec3(0.080, 0.010, 0.005) * blood;

    float carotene = skin.pigments.z - 0.35;
    colour += vec3(0.040, 0.028, -0.012) * carotene;

    float temperature = skin.physiology.z - 0.50;
    colour += vec3(0.026, 0.004, -0.018) * temperature;
    return clamp(colour, vec3(0.015, 0.010, 0.008), vec3(0.95, 0.90, 0.85));
}

float lsg_skin_base_roughness() {
    return clamp(0.62 + (skin.surface.z - 0.5) * 0.26
                     - skin.surface.x * 0.16
                     - skin.surface.y * 0.05
                     - skin.physiology.y * 0.10
                     + skin.pigments.w * 0.035,
                 0.24, 0.90);
}

float lsg_skin_specular_scale() {
    return 0.80 + clamp(skin.surface.w, 0.0, 1.0) * 0.65;
}

#endif
