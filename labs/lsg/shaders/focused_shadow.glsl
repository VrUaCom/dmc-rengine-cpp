// Shared cinematic receiver-plane PCF. Depths remain unfiltered: compare each
// texel first, then filter visibility. No random/screen-space jitter.
#ifndef LSG_FOCUSED_SHADOW_GLSL
#define LSG_FOCUSED_SHADOW_GLSL

vec2 lsg_receiver_depth_gradient(vec3 dx, vec3 dy) {
    float det = dx.x * dy.y - dx.y * dy.x;
    float scale = max(length(dx.xy) * length(dy.xy), 1e-20);
    if (abs(det) <= 1e-5 * scale) return vec2(0.0);
    vec2 gradient = vec2(dy.y * dx.z - dx.y * dy.z,
                         dx.x * dy.z - dy.x * dx.z) / det;
    if (any(isnan(gradient)) || any(isinf(gradient))) return vec2(0.0);
    return clamp(gradient, vec2(-64.0), vec2(64.0));
}

float lsg_tent_weight(float delta_texels) {
    return max(2.5 - abs(delta_texels), 0.0);
}

float lsg_shadow_tap_margin(sampler2D depths, vec3 coord, ivec2 tap,
                            ivec2 size_px, vec2 gradient, float bias,
                            float max_correction) {
    // Missing coverage is lit, matching the existing shadow border contract.
    if (any(lessThan(tap, ivec2(0))) ||
        any(greaterThanEqual(tap, size_px))) return 1.0;
    vec2 tap_uv = (vec2(tap) + 0.5) / vec2(size_px);
    float correction = clamp(dot(gradient, tap_uv - coord.xy),
                             -max_correction, max_correction);
    return texelFetch(depths, tap, 0).r - (coord.z + correction - bias);
}

bool lsg_shadow_outside(vec3 coord) {
    return any(lessThanEqual(coord, vec3(0.0))) ||
           any(greaterThanEqual(coord, vec3(1.0)));
}

float lsg_cinematic_shadow_visibility(sampler2D depths, vec3 coord,
                                      float bias, float max_correction) {
    // Evaluate derivatives before the per-fragment coverage branch.
    vec2 gradient = lsg_receiver_depth_gradient(dFdx(coord), dFdy(coord));
    if (lsg_shadow_outside(coord)) return 1.0;
    ivec2 size_px = textureSize(depths, 0);
    vec2 pixel = coord.xy * vec2(size_px);
    ivec2 center = ivec2(floor(pixel));
    float visible = 0.0, total_weight = 0.0;
    for (int y = -2; y <= 2; ++y) for (int x = -2; x <= 2; ++x) {
        ivec2 tap = center + ivec2(x, y);
        vec2 delta = vec2(tap) + 0.5 - pixel;
        float weight = lsg_tent_weight(delta.x) * lsg_tent_weight(delta.y);
        float margin = lsg_shadow_tap_margin(depths, coord, tap, size_px,
                                            gradient, bias, max_correction);
        visible += weight * (margin >= 0.0 ? 1.0 : 0.0);
        total_weight += weight;
    }
    return visible / max(total_weight, 1e-6);
}

float lsg_cinematic_shadow_margin(sampler2D depths, vec3 coord,
                                  float bias, float max_correction) {
    vec2 gradient = lsg_receiver_depth_gradient(dFdx(coord), dFdy(coord));
    if (lsg_shadow_outside(coord)) return 1.0;
    ivec2 size_px = textureSize(depths, 0);
    ivec2 tap = ivec2(floor(coord.xy * vec2(size_px)));
    return lsg_shadow_tap_margin(depths, coord, tap, size_px,
                                gradient, bias, max_correction);
}
#endif
