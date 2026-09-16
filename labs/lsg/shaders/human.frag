#version 450

layout(location = 0) in vec3 surface_position_m;
layout(location = 1) in vec3 view_normal;
layout(location = 2) flat in uint body_region;
layout(location = 3) in vec3 view_position_m;
layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform LsgPush {
    vec4 center_units;
    vec4 camera;
    vec4 geometry0;
    vec4 geometry1;
    vec4 skin0;
    vec4 micro0;
    vec4 render;
    uvec4 flags;
} pc;

uint pcg_hash(uint input_value) {
    uint state = input_value * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

uint hash_cell(ivec3 cell, uint seed) {
    uint h = pcg_hash(seed ^ uint(cell.x) * 0x9E3779B9u);
    h = pcg_hash(h ^ uint(cell.y) * 0x85EBCA6Bu);
    h = pcg_hash(h ^ uint(cell.z) * 0xC2B2AE35u);
    h = pcg_hash(h ^ body_region * 0x27D4EB2Du);
    return h;
}

float hash01(uint h) { return float(h >> 8u) * (1.0 / 16777216.0); }
float smooth01(float x) { return x * x * (3.0 - 2.0 * x); }

float value_noise(vec3 p, uint seed) {
    ivec3 c = ivec3(floor(p));
    vec3 f = fract(p);
    f = vec3(smooth01(f.x), smooth01(f.y), smooth01(f.z));
    float v000 = hash01(hash_cell(c + ivec3(0,0,0), seed));
    float v100 = hash01(hash_cell(c + ivec3(1,0,0), seed));
    float v010 = hash01(hash_cell(c + ivec3(0,1,0), seed));
    float v110 = hash01(hash_cell(c + ivec3(1,1,0), seed));
    float v001 = hash01(hash_cell(c + ivec3(0,0,1), seed));
    float v101 = hash01(hash_cell(c + ivec3(1,0,1), seed));
    float v011 = hash01(hash_cell(c + ivec3(0,1,1), seed));
    float v111 = hash01(hash_cell(c + ivec3(1,1,1), seed));
    float x00 = mix(v000, v100, f.x), x10 = mix(v010, v110, f.x);
    float x01 = mix(v001, v101, f.x), x11 = mix(v011, v111, f.x);
    return mix(mix(x00, x10, f.y), mix(x01, x11, f.y), f.z);
}

int detail_band(float mm_per_pixel) {
    if (mm_per_pixel < 0.10) return 3;
    if (mm_per_pixel < 1.00) return 2;
    if (mm_per_pixel < 3.00) return 1;
    return 0;
}

float region_density_scale(uint region) {
    if (region == 0u) return 1.18;
    if (region == 7u) return 1.08;
    if (region == 1u || region == 2u) return 0.98;
    if (region == 9u || region == 10u) return 0.78;
    return 0.88;
}

bool inside_box(vec2 uv, vec2 lo, vec2 hi) {
    return all(greaterThanEqual(uv, lo)) && all(lessThanEqual(uv, hi));
}

bool hud_glyph(uint button, vec2 uv) {
    vec2 q = abs(uv - vec2(0.5));
    if (button == 0u) {
        bool outer = q.x <= 0.16 && q.y <= 0.28;
        bool inner = q.x <= 0.075 && q.y <= 0.18;
        return outer && !inner;
    }
    if (button == 1u) {
        return inside_box(uv, vec2(0.47, 0.22), vec2(0.55, 0.78)) ||
               inside_box(uv, vec2(0.39, 0.70), vec2(0.53, 0.79)) ||
               inside_box(uv, vec2(0.37, 0.20), vec2(0.65, 0.29));
    }
    bool left = inside_box(uv, vec2(0.34, 0.22), vec2(0.42, 0.78));
    bool top = inside_box(uv, vec2(0.39, 0.70), vec2(0.58, 0.78));
    bool bottom = inside_box(uv, vec2(0.39, 0.22), vec2(0.58, 0.30));
    bool right = inside_box(uv, vec2(0.56, 0.30), vec2(0.64, 0.70));
    return left || top || bottom || right;
}

vec3 perturb_normal(vec3 n, vec3 view_pos, float height_field, float strength) {
    vec3 dpdx = dFdx(view_pos);
    vec3 dpdy = dFdy(view_pos);
    float dhdx = dFdx(height_field);
    float dhdy = dFdy(height_field);
    vec3 r1 = cross(dpdy, n);
    vec3 r2 = cross(n, dpdx);
    float det = dot(dpdx, r1);
    if (abs(det) < 1e-8) return n;
    vec3 gradient = sign(det) * (dhdx * r1 + dhdy * r2);
    return normalize(abs(det) * n - gradient * strength);
}

float distribution_ggx(float ndoth, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float d = ndoth * ndoth * (a2 - 1.0) + 1.0;
    return a2 / max(3.14159265 * d * d, 1e-5);
}

float geometry_schlick(float ndotv, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) * 0.125;
    return ndotv / max(ndotv * (1.0 - k) + k, 1e-5);
}

vec3 fresnel_schlick(float cos_theta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

void main() {
    uint profile_index = pc.flags.x & 1u;
    bool detail_enabled = pc.flags.y != 0u;

    if (pc.flags.w != 0u) {
        uint button = body_region >= 100u ? body_region - 100u : 0u;
        vec2 uv = surface_position_m.xy;
        bool selected = (button == 0u && profile_index == 0u) ||
                        (button == 1u && profile_index == 1u) ||
                        (button == 2u && detail_enabled);
        float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        vec3 panel = selected ? vec3(0.16, 0.56, 0.92) : vec3(0.12, 0.16, 0.22);
        if (edge < 0.055) panel = selected ? vec3(0.45, 0.78, 1.00) : vec3(0.38, 0.43, 0.50);
        if (hud_glyph(button, uv)) panel = vec3(0.96, 0.98, 1.00);
        out_colour = vec4(panel, 1.0);
        return;
    }

    uint seed = floatBitsToUint(pc.render.w);
    float footprint_m = max(length(dFdx(surface_position_m)), length(dFdy(surface_position_m)));
    int band = detail_band(footprint_m * 1000.0);

    float melanin = clamp(pc.skin0.x, 0.0, 1.0);
    float haemoglobin = clamp(pc.skin0.y, 0.0, 1.0);
    float oiliness = clamp(pc.skin0.z, 0.0, 1.0);
    float hydration = clamp(pc.skin0.w, 0.0, 1.0);

    vec3 light_skin = vec3(0.66, 0.39, 0.29);
    vec3 dark_skin = vec3(0.12, 0.050, 0.030);
    vec3 base_colour = mix(light_skin, dark_skin, pow(melanin, 0.82) * 0.90);
    base_colour += vec3(0.08, 0.010, 0.005) * (haemoglobin - 0.45);

    float meso = 0.0;
    float pore_mask = 0.0;
    float height_field = 0.0;
    float roughness = clamp(0.62 + (pc.micro0.x - 0.5) * 0.26 - oiliness * 0.16 - hydration * 0.05,
                            0.28, 0.86);

    if (detail_enabled && band >= 1) {
        meso = value_noise(surface_position_m / 0.0065, seed ^ 0xA511E9B3u) - 0.5;
        float vascular = value_noise(surface_position_m / 0.032, seed ^ 0x63D83595u) - 0.5;
        base_colour += vec3(0.055, -0.005, -0.010) * vascular * haemoglobin;
        base_colour *= 1.0 + meso * 0.055;
        roughness = clamp(roughness + meso * 0.07, 0.24, 0.90);
        height_field += meso * 0.000055;
    }

    if (detail_enabled && band >= 2) {
        float cell_m = mix(0.00052, 0.00024, clamp(pc.micro0.z, 0.0, 1.0));
        float pore_noise = value_noise(surface_position_m / cell_m, seed ^ 0xB5297A4Du);
        float density = clamp(pc.micro0.y * region_density_scale(body_region), 0.05, 0.95);
        float threshold = mix(0.96, 0.72, density);
        pore_mask = smoothstep(threshold, min(0.995, threshold + 0.16), pore_noise);
        float depth = mix(0.000010, 0.000050, clamp(pc.micro0.w, 0.0, 1.0));
        height_field -= pore_mask * depth;
        roughness = clamp(roughness + pore_mask * 0.10, 0.25, 0.95);
    }

    vec3 n = normalize(view_normal);
    if (detail_enabled && band >= 1) n = perturb_normal(n, view_position_m, height_field, 2.2);
    vec3 v = normalize(-view_position_m);
    vec3 l = normalize(vec3(-0.30, 0.62, 0.72));
    vec3 h = normalize(v + l);
    float ndotl = max(dot(n, l), 0.0);
    float ndotv = max(dot(n, v), 0.001);
    float ndoth = max(dot(n, h), 0.0);
    float hdotv = max(dot(h, v), 0.0);

    const float skin_f0_scalar = 0.0277778; // ((1.4 - 1) / (1.4 + 1))^2
    vec3 f0 = vec3(skin_f0_scalar);
    vec3 fresnel = fresnel_schlick(hdotv, f0);
    float d = distribution_ggx(ndoth, roughness);
    float g = geometry_schlick(ndotv, roughness) * geometry_schlick(max(ndotl, 0.001), roughness);
    vec3 specular = fresnel * (d * g / max(4.0 * ndotv * max(ndotl, 0.001), 0.001));

    float wrapped = clamp((ndotl + 0.22) / 1.22, 0.0, 1.0);
    vec3 diffuse = base_colour * wrapped * (1.0 - fresnel) / 3.14159265;
    vec3 subsurface_approx = base_colour * vec3(1.05, 0.45, 0.32) * pow(1.0 - ndotl, 2.0) * 0.045;
    vec3 ambient = base_colour * 0.15;
    vec3 colour = ambient + diffuse * 2.6 + specular * (1.2 + oiliness * 0.75) + subsurface_approx;
    out_colour = vec4(max(colour, vec3(0.0)), 1.0);
}
