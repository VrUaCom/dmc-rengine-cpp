#ifndef RENGINE_LSG_SKIN_TRANSPORT_GLSL
#define RENGINE_LSG_SKIN_TRANSPORT_GLSL

vec3 lsg_skin_subsurface(vec3 base_colour, float signed_ndotl, float ndotl, vec3 sky) {
    float wrap = smoothstep(-0.04, 0.28, signed_ndotl);
    float terminator = (1.0 - ndotl) * wrap;
    float strength = clamp(skin.physiology.w, 0.0, 1.0);
    vec3 spectral_bias = mix(vec3(0.92, 0.50, 0.40),
                             vec3(1.08, 0.43, 0.30),
                             clamp(skin.pigments.y, 0.0, 1.0));
    return base_colour * spectral_bias *
           terminator * mix(0.008, 0.030, strength) *
           (0.55 + 0.45 * sky);
}

#endif
