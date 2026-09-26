#version 450
#extension GL_GOOGLE_include_directive : require
#include "focused_shadow.glsl"

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

layout(set = 0, binding = 0, std140) uniform FrameLighting {
    vec4 sun_direction_intensity;
    vec4 sun_tint_sky_intensity;
    vec4 sky_zenith_exposure;
    vec4 sky_horizon_scene_lum;
    vec4 filter_tint_transmission;
    vec4 eye_filter_misc;
    uvec4 modes;
    vec4 face0;
    vec4 face1;
    vec4 face2;
    vec4 face3;
    vec4 face4;
} lighting;

layout(set = 0, binding = 1) uniform sampler2D shadow_depth;
layout(set = 0, binding = 2) uniform sampler2D self_shadow_depth;

uint diagnostic_mode() { return (pc.flags.w >> 1u) & 3u; }
uint surface_diagnostic_mode() { return (pc.flags.w >> 20u) & 7u; }
uint close_shadow_level() { return (pc.flags.w >> 23u) & 3u; }

float focused_shadow_half_extent() {
    uint level = close_shadow_level();
    return level == 2u ? 0.55 : level == 1u ? 0.85 : 1.20;
}

float focused_shadow_depth_half_extent() {
    uint level = close_shadow_level();
    return level == 2u ? 1.10 : level == 1u ? 1.50 : 2.00;
}

float snap_shadow_axis(float value, float texel_size) {
    return floor(value / max(texel_size, 1e-7) + 0.5) * texel_size;
}
bool ui_environment_pass() { return (pc.flags.w & 1u) != 0u; }

uint pcg_hash(uint input_value) {
    uint state = input_value * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

uint hash_cell(ivec3 cell, uint seed_key) {
    // Surface noise is anchored only in deterministic object-space cells.
    // BodyRegion is intentionally excluded so semantic region boundaries cannot
    // introduce a discontinuous phase jump in meso/micro detail.
    uint h = pcg_hash(seed_key);
    h = pcg_hash(h ^ uint(cell.x));
    h = pcg_hash(h ^ uint(cell.y));
    h = pcg_hash(h ^ uint(cell.z));
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

float smooth_range(float value, float lo, float hi) {
    return smoothstep(lo, hi, value);
}

float smooth_band(float value, float rise0, float rise1,
                  float fall0, float fall1) {
    return smooth_range(value, rise0, rise1) *
           (1.0 - smooth_range(value, fall0, fall1));
}

float anatomical_pore_density_scale(vec3 p) {
    // Continuous v0 field replacing hard BodyRegion material jumps.
    // Coordinates are deformed object-space metres around the body centre.
    float y01 = clamp(p.y / 1.75 + 0.5, 0.0, 1.0);
    float lateral = clamp(abs(p.x) / 0.52, 0.0, 1.0);

    float head = smooth_range(y01, 0.80, 0.90);
    float neck = smooth_band(y01, 0.75, 0.81, 0.86, 0.91);
    float upper_torso = smooth_band(y01, 0.56, 0.64, 0.76, 0.83);
    float arms = smooth_band(y01, 0.47, 0.57, 0.76, 0.85) *
                 smooth_range(lateral, 0.44, 0.76);
    float lower_leg = smooth_band(y01, 0.06, 0.12, 0.27, 0.34);
    float foot = 1.0 - smooth_range(y01, 0.05, 0.11);

    float scale = 0.88;
    scale += head * 0.30;
    scale += neck * 0.08;
    scale += upper_torso * 0.05;
    scale -= arms * 0.05;
    scale -= lower_leg * 0.07;
    scale -= foot * 0.10;
    return clamp(scale, 0.76, 1.20);
}

vec2 pore_field(vec3 p, uint seed, float cell_m, float density, float depth_m) {
    ivec3 base = ivec3(floor(p / cell_m));
    float pore_height = 0.0;
    float pore_influence = 0.0;
    for (int z = -1; z <= 1; ++z) {
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                ivec3 cell = base + ivec3(x, y, z);
                uint h = hash_cell(cell, seed ^ 0xB5297A4Du);
                if (hash01(pcg_hash(h ^ 0xD1B54A35u)) >= density) continue;

                vec3 jitter = vec3(
                    0.15 + 0.70 * hash01(pcg_hash(h ^ 0x68E31DA4u)),
                    0.15 + 0.70 * hash01(pcg_hash(h ^ 0xB5297A4Du)),
                    0.15 + 0.70 * hash01(pcg_hash(h ^ 0x1B56C4E9u)));
                vec3 center = (vec3(cell) + jitter) * cell_m;

                vec3 axis = vec3(
                    hash01(pcg_hash(h ^ 0x9E3779B9u)) * 2.0 - 1.0,
                    hash01(pcg_hash(h ^ 0x85EBCA6Bu)) * 2.0 - 1.0,
                    hash01(pcg_hash(h ^ 0xC2B2AE35u)) * 2.0 - 1.0);
                float axis_length = length(axis);
                axis = axis_length > 1e-6 ? axis / axis_length : vec3(0.0, 1.0, 0.0);

                vec3 delta = p - center;
                float axial = dot(delta, axis);
                vec3 tangential = delta - axis * axial;
                float axial_scale = 0.85 + 0.30 * hash01(pcg_hash(h ^ 0x27D4EB2Du));
                float distance_to_pore = length(vec4(tangential, axial / axial_scale));
                float radius = cell_m * mix(0.16, 0.34, hash01(pcg_hash(h ^ 0x165667B1u)));
                if (distance_to_pore >= radius) continue;

                float t = clamp(1.0 - distance_to_pore / radius, 0.0, 1.0);
                float shape = smooth01(t);
                float local_depth = depth_m * mix(0.55, 1.0, hash01(pcg_hash(h ^ 0xA511E9B3u)));
                pore_height = min(pore_height, -local_depth * shape);
                pore_influence = max(pore_influence, shape);
            }
        }
    }
    return vec2(pore_height, pore_influence);
}

bool inside_box(vec2 uv, vec2 lo, vec2 hi) {
    return all(greaterThanEqual(uv, lo)) && all(lessThanEqual(uv, hi));
}

bool hud_glyph(uint button, vec2 uv) {
    vec2 q = abs(uv - vec2(0.5));
    if (button == 0u) {
        bool outer = q.x <= 0.15 && q.y <= 0.27;
        bool inner = q.x <= 0.065 && q.y <= 0.17;
        return outer && !inner;
    }
    if (button == 1u) {
        return inside_box(uv, vec2(0.47, 0.22), vec2(0.55, 0.78)) ||
               inside_box(uv, vec2(0.39, 0.69), vec2(0.53, 0.79)) ||
               inside_box(uv, vec2(0.37, 0.20), vec2(0.65, 0.29));
    }
    if (button == 2u) {
        bool left = inside_box(uv, vec2(0.34, 0.22), vec2(0.42, 0.78));
        bool top = inside_box(uv, vec2(0.39, 0.70), vec2(0.58, 0.78));
        bool bottom = inside_box(uv, vec2(0.39, 0.22), vec2(0.58, 0.30));
        bool right = inside_box(uv, vec2(0.56, 0.30), vec2(0.64, 0.70));
        return left || top || bottom || right;
    }
    if (button == 3u) {
        float head = length(uv - vec2(0.50, 0.76));
        bool spine = inside_box(uv, vec2(0.47, 0.27), vec2(0.53, 0.67));
        bool shoulders = inside_box(uv, vec2(0.28, 0.54), vec2(0.72, 0.60));
        bool pelvis = inside_box(uv, vec2(0.36, 0.28), vec2(0.64, 0.34));
        return head <= 0.095 || spine || shoulders || pelvis;
    }
    if (button == 4u) {
        bool body = inside_box(uv, vec2(0.27, 0.33), vec2(0.67, 0.67));
        float lens = length(uv - vec2(0.70, 0.50));
        bool grip = inside_box(uv, vec2(0.38, 0.65), vec2(0.52, 0.74));
        return body || lens <= 0.16 || grip;
    }
    if (button == 5u) {
        bool horizontal = inside_box(uv, vec2(0.22, 0.47), vec2(0.78, 0.53));
        bool vertical = inside_box(uv, vec2(0.47, 0.22), vec2(0.53, 0.78));
        float center = length(uv - vec2(0.5));
        return horizontal || vertical || (center > 0.22 && center < 0.28);
    }
    if (button == 6u) {
        float ring = length(uv - vec2(0.50, 0.50));
        bool arc = ring > 0.22 && ring < 0.30 && !(uv.x > 0.56 && uv.y > 0.58);
        bool arrow = inside_box(uv, vec2(0.60, 0.58), vec2(0.78, 0.66)) ||
                     inside_box(uv, vec2(0.70, 0.50), vec2(0.78, 0.66));
        return arc || arrow;
    }
    if (button == 7u) {
        // Physiology: compact heart/pulse icon.
        bool left_lobe = length(uv - vec2(0.40, 0.60)) < 0.15;
        bool right_lobe = length(uv - vec2(0.60, 0.60)) < 0.15;
        bool lower = uv.y < 0.62 && abs(uv.x - 0.50) < (0.34 - 0.42 * (0.62 - uv.y));
        bool pulse = inside_box(uv, vec2(0.23, 0.45), vec2(0.40, 0.50)) ||
                     inside_box(uv, vec2(0.39, 0.36), vec2(0.45, 0.58)) ||
                     inside_box(uv, vec2(0.44, 0.47), vec2(0.58, 0.52)) ||
                     inside_box(uv, vec2(0.57, 0.42), vec2(0.63, 0.58)) ||
                     inside_box(uv, vec2(0.62, 0.47), vec2(0.78, 0.52));
        return left_lobe || right_lobe || lower || pulse;
    }

    if (button == 8u) {
        // Eyes: almond/ring outline plus pupil.
        vec2 e = (uv - vec2(0.50)) / vec2(0.34, 0.20);
        float ellipse = dot(e, e);
        bool eye_outline = ellipse > 0.72 && ellipse < 1.08;
        bool pupil = length(uv - vec2(0.50)) < 0.085;
        return eye_outline || pupil;
    }

    if (button == 9u) {
        // Time: compact sun disk with four rays.
        float r = length(uv - vec2(0.50));
        bool disk = r < 0.14;
        bool ring = r > 0.20 && r < 0.25;
        bool rays = inside_box(uv, vec2(0.47, 0.18), vec2(0.53, 0.31)) ||
                    inside_box(uv, vec2(0.47, 0.69), vec2(0.53, 0.82)) ||
                    inside_box(uv, vec2(0.18, 0.47), vec2(0.31, 0.53)) ||
                    inside_box(uv, vec2(0.69, 0.47), vec2(0.82, 0.53));
        return disk || ring || rays;
    }

    if (button == 11u) {
        return inside_box(uv, vec2(0.28, 0.68), vec2(0.72, 0.78)) ||
               inside_box(uv, vec2(0.45, 0.22), vec2(0.55, 0.73));
    }
    // Filter: optical pane with diagonal polarization marks.
    bool pane = (q.x > 0.20 && q.x < 0.27 && q.y < 0.30) ||
                (q.y > 0.23 && q.y < 0.30 && q.x < 0.27);
    bool slash0 = abs((uv.y - 0.30) - (uv.x - 0.30)) < 0.045 &&
                  uv.x > 0.28 && uv.x < 0.58 && uv.y > 0.28 && uv.y < 0.58;
    bool slash1 = abs((uv.y - 0.45) - (uv.x - 0.45)) < 0.045 &&
                  uv.x > 0.43 && uv.x < 0.72 && uv.y > 0.43 && uv.y < 0.72;
    return pane || slash0 || slash1;
}

uint font_bits(uint c) {
    if (c == 48u) return 0x69BD96u;
    if (c == 49u) return 0xF66676u;
    if (c == 65u) return 0x99F996u;
    if (c == 66u) return 0x799797u;
    if (c == 67u) return 0xE1111Eu;
    if (c == 68u) return 0x799997u;
    if (c == 69u) return 0xF1171Fu;
    if (c == 70u) return 0x11171Fu;
    if (c == 71u) return 0xE99D1Eu;
    if (c == 72u) return 0x999F99u;
    if (c == 73u) return 0xF6666Fu;
    if (c == 74u) return 0x69888Cu;
    if (c == 75u) return 0x995359u;
    if (c == 76u) return 0xF11111u;
    if (c == 77u) return 0x999FF9u;
    if (c == 78u) return 0x999DB9u;
    if (c == 79u) return 0x699996u;
    if (c == 80u) return 0x111797u;
    if (c == 81u) return 0xED9996u;
    if (c == 82u) return 0x995797u;
    if (c == 83u) return 0x78861Eu;
    if (c == 84u) return 0x66666Fu;
    if (c == 85u) return 0x699999u;
    if (c == 86u) return 0x669999u;
    if (c == 87u) return 0x9FF999u;
    if (c == 88u) return 0x996699u;
    if (c == 89u) return 0x666699u;
    if (c == 90u) return 0xF1248Fu;
    return 0u;
}

bool font_pixel(uint c, vec2 uv) {
    if (c == 32u) return false;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0)))) return false;
    uint col = min(uint(floor(uv.x * 4.0)), 3u);
    uint row = min(uint(floor((1.0 - uv.y) * 6.0)), 5u);
    uint bit_index = row * 4u + col;
    return ((font_bits(c) >> bit_index) & 1u) != 0u;
}

bool shadow_probe_active() { return (pc.flags.w & (1u << 25u)) != 0u; }
uint probe_label() { return shadow_probe_active() ? 1u + ((pc.flags.w >> 20u) & 7u) : 0u; }
uint probe_label_char(uint index) {
    const uint p0[22] = uint[22](84u,69u,83u,84u,32u,32u,70u,73u,86u,69u,32u,70u,73u,88u,69u,68u,32u,86u,73u,69u,87u,83u);
    if (probe_label() == 0u && index < 22u) return p0[index];
    const uint p1[12] = uint[12](84u,69u,83u,84u,32u,32u,78u,79u,82u,77u,65u,76u);
    if (probe_label() == 1u && index < 12u) return p1[index];
    const uint p2[16] = uint[16](84u,69u,83u,84u,32u,32u,86u,73u,83u,73u,66u,73u,76u,73u,84u,89u);
    if (probe_label() == 2u && index < 16u) return p2[index];
    const uint p3[13] = uint[13](84u,69u,83u,84u,32u,32u,78u,79u,82u,77u,65u,76u,83u);
    if (probe_label() == 3u && index < 13u) return p3[index];
    const uint p4[13] = uint[13](84u,69u,83u,84u,32u,32u,82u,69u,71u,73u,79u,78u,83u);
    if (probe_label() == 4u && index < 13u) return p4[index];
    const uint p5[13] = uint[13](84u,69u,83u,84u,32u,32u,67u,79u,77u,80u,65u,82u,69u);
    if (probe_label() == 5u && index < 13u) return p5[index];
    return 32u;
}
uint probe_label_length() {
    if (probe_label() == 0u) return 22u;
    if (probe_label() == 1u) return 12u;
    if (probe_label() == 2u) return 16u;
    if (probe_label() == 3u) return 13u;
    if (probe_label() == 4u) return 13u;
    if (probe_label() == 5u) return 13u;
    return 0u;
}

uint tooltip_length(uint tooltip) {
    if (tooltip == 11u) return probe_label_length();
    if (tooltip == 0u) return 20u;
    if (tooltip == 1u) return 22u;
    if (tooltip == 2u) return 23u;
    if (tooltip == 3u) return 21u;
    if (tooltip == 4u) return 21u;
    if (tooltip == 5u) return 20u;
    if (tooltip == 6u) return 20u;
    if (tooltip == 7u) return 22u;
    if (tooltip == 8u) return 16u;
    if (tooltip == 9u) return 20u;
    if (tooltip == 10u) return 20u;
    return 0u;
}

uint tooltip_char(uint tooltip, uint index) {
    if (tooltip == 11u) return probe_label_char(index);
    const uint t0[20] = uint[20](67u,72u,65u,82u,32u,48u,32u,32u,77u,65u,76u,69u,32u,80u,82u,79u,70u,73u,76u,69u);
    const uint t1[22] = uint[22](67u,72u,65u,82u,32u,49u,32u,32u,70u,69u,77u,65u,76u,69u,32u,80u,82u,79u,70u,73u,76u,69u);
    const uint t2[23] = uint[23](68u,69u,84u,65u,73u,76u,32u,32u,80u,82u,79u,67u,69u,68u,85u,82u,65u,76u,32u,83u,75u,73u,78u);
    const uint t3[21] = uint[21](83u,75u,69u,76u,69u,84u,79u,78u,32u,32u,74u,79u,73u,78u,84u,32u,68u,69u,66u,85u,71u);
    const uint t4[21] = uint[21](67u,65u,77u,69u,82u,65u,32u,32u,67u,89u,67u,76u,69u,32u,80u,82u,69u,83u,69u,84u,83u);
    const uint t5[20] = uint[20](68u,73u,65u,71u,32u,32u,82u,65u,87u,32u,86u,73u,69u,87u,32u,77u,79u,68u,69u,83u);
    const uint t6[20] = uint[20](82u,69u,83u,69u,84u,32u,32u,67u,69u,78u,84u,69u,82u,32u,67u,65u,77u,69u,82u,65u);
    const uint t7[22] = uint[22](80u,72u,89u,83u,32u,32u,67u,89u,67u,76u,69u,32u,66u,79u,68u,89u,32u,83u,84u,65u,84u,69u);
    const uint t8[16] = uint[16](69u,89u,69u,83u,32u,32u,67u,89u,67u,76u,69u,32u,86u,73u,69u,87u);
    const uint t9[20] = uint[20](84u,73u,77u,69u,32u,32u,67u,89u,67u,76u,69u,32u,68u,65u,89u,76u,73u,71u,72u,84u);
    const uint t10[20] = uint[20](70u,73u,76u,84u,69u,82u,32u,32u,67u,89u,67u,76u,69u,32u,79u,80u,84u,73u,67u,83u);
    if (tooltip == 0u && index < 20u) return t0[index];
    if (tooltip == 1u && index < 22u) return t1[index];
    if (tooltip == 2u && index < 23u) return t2[index];
    if (tooltip == 3u && index < 21u) return t3[index];
    if (tooltip == 4u && index < 21u) return t4[index];
    if (tooltip == 5u && index < 20u) return t5[index];
    if (tooltip == 6u && index < 20u) return t6[index];
    if (tooltip == 7u && index < 22u) return t7[index];
    if (tooltip == 8u && index < 16u) return t8[index];
    if (tooltip == 9u && index < 20u) return t9[index];
    if (tooltip == 10u && index < 20u) return t10[index];
    return 32u;
}

bool character_menu_open() { return (pc.flags.w & (1u << 26u)) != 0u; }

uint character_menu_char(uint option, uint index) {
    const uint base_label[4] = uint[4](66u,65u,83u,69u);
    const uint female_label[6] = uint[6](70u,69u,77u,65u,76u,69u);
    const uint ada_label[3] = uint[3](65u,68u,65u);
    if (option == 0u && index < 4u) return base_label[index];
    if (option == 1u && index < 6u) return female_label[index];
    if (option == 2u && index < 3u) return ada_label[index];
    return 32u;
}

uint character_menu_length(uint option) {
    return option == 0u ? 4u : (option == 1u ? 6u : 3u);
}

bool character_menu_text_pixel(uint option, vec2 uv) {
    uint length = character_menu_length(option);
    const float advance = 0.110;
    const float glyph_width = 0.082;
    const float glyph_y0 = 0.23;
    const float glyph_height = 0.54;
    float text_width = float(length) * advance;
    float start_x = 0.5 - text_width * 0.5;
    if (uv.x < start_x || uv.x >= start_x + text_width ||
        uv.y < glyph_y0 || uv.y >= glyph_y0 + glyph_height) return false;
    uint index = uint(floor((uv.x - start_x) / advance));
    if (index >= length) return false;
    float char_start = start_x + float(index) * advance;
    vec2 glyph_uv = vec2((uv.x - char_start) / glyph_width,
                         (uv.y - glyph_y0) / glyph_height);
    return font_pixel(character_menu_char(option, index), glyph_uv);
}

bool tooltip_text_pixel(uint tooltip, vec2 uv) {
    uint length = tooltip_length(tooltip);
    if (length == 0u) return false;

    const float advance = 0.0410;
    const float glyph_width = 0.0320;
    const float glyph_y0 = 0.24;
    const float glyph_height = 0.52;
    float text_width = float(length) * advance;
    float start_x = 0.5 - text_width * 0.5;
    if (uv.x < start_x || uv.x >= start_x + text_width ||
        uv.y < glyph_y0 || uv.y >= glyph_y0 + glyph_height) return false;

    uint index = uint(floor((uv.x - start_x) / advance));
    if (index >= length) return false;
    float char_start = start_x + float(index) * advance;
    vec2 glyph_uv = vec2((uv.x - char_start) / glyph_width,
                         (uv.y - glyph_y0) / glyph_height);
    return font_pixel(tooltip_char(tooltip, index), glyph_uv);
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

vec3 rotate_y(vec3 v, float a) {
    float c = cos(a), s = sin(a);
    return vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

vec3 rotate_x(vec3 v, float a) {
    float c = cos(a), s = sin(a);
    return vec3(v.x, c * v.y - s * v.z, s * v.y + c * v.z);
}

vec3 aces_fitted(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 bulk_filter_rgb() {
    return clamp(lighting.filter_tint_transmission.rgb, vec3(0.0), vec3(1.0)) *
           clamp(lighting.filter_tint_transmission.w, 0.0, 1.0);
}

float rayleigh_dolp_from_mu(float mu) {
    mu = clamp(mu, -1.0, 1.0);
    float mu2 = mu * mu;
    return clamp((1.0 - mu2) / max(1.0 + mu2, 1e-6), 0.0, 1.0);
}

float polarized_attenuation(float dolp, float axis_alignment_sq, float strength) {
    dolp = clamp(dolp, 0.0, 1.0);
    axis_alignment_sq = clamp(axis_alignment_sq, 0.0, 1.0);
    strength = clamp(strength, 0.0, 1.0);
    return clamp(1.0 - 0.55 * strength * dolp * (1.0 - axis_alignment_sq),
                 0.45, 1.0);
}

float sky_polarization_attenuation(vec3 ray_world, vec3 sun_world) {
    float strength = clamp(lighting.eye_filter_misc.y, 0.0, 1.0);
    if (strength <= 0.0) return 1.0;

    vec3 pol = cross(ray_world, sun_world);
    vec3 axis = vec3(0.0, 1.0, 0.0) -
                ray_world * dot(vec3(0.0, 1.0, 0.0), ray_world);
    float pol_len2 = dot(pol, pol);
    float axis_len2 = dot(axis, axis);
    if (pol_len2 < 1e-8 || axis_len2 < 1e-8) return 1.0;

    pol *= inversesqrt(pol_len2);
    axis *= inversesqrt(axis_len2);
    float alignment_sq = pow(clamp(dot(pol, axis), -1.0, 1.0), 2.0);
    float mu = clamp(dot(ray_world, sun_world), -1.0, 1.0);
    return polarized_attenuation(rayleigh_dolp_from_mu(mu),
                                 alignment_sq, strength);
}

vec3 sun_world_direction() {
    return normalize(lighting.sun_direction_intensity.xyz);
}

vec3 shadow_coord_from_world(vec3 world_position, bool focused) {
    vec3 forward = normalize(-sun_world_direction());
    vec3 reference_up = abs(forward.y) > 0.95 ? vec3(0.0, 0.0, 1.0)
                                              : vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(reference_up, forward));
    vec3 up = normalize(cross(forward, right));

    float half_extent = focused ? focused_shadow_half_extent() : 5.50;
    float depth_half_extent =
        focused ? focused_shadow_depth_half_extent() : 6.00;

    vec3 focus_center = focused ? vec3(0.0, pc.camera.w, 0.0) : vec3(0.0);
    float center_right = dot(focus_center, right);
    float center_up = dot(focus_center, up);
    float center_forward = dot(focus_center, forward);

    if (focused) {
        const float shadow_map_size = 2048.0;
        float world_texel = (2.0 * half_extent) / shadow_map_size;
        center_right = snap_shadow_axis(center_right, world_texel);
        center_up = snap_shadow_axis(center_up, world_texel);
    }

    vec3 coord;
    coord.x = (dot(world_position, right) - center_right) /
              half_extent * 0.5 + 0.5;
    coord.y = (dot(world_position, up) - center_up) /
              half_extent * 0.5 + 0.5;
    coord.z = ((dot(world_position, forward) - center_forward) +
               depth_half_extent) / (2.0 * depth_half_extent);
    return coord;
}

vec3 view_to_world_direction(vec3 view_direction) {
    return normalize(rotate_y(rotate_x(view_direction, pc.camera.y), pc.camera.x));
}

float coarse_shadow_visibility(vec3 world_position, vec3 normal_view, vec3 light_view) {
    vec3 n_world = view_to_world_direction(normalize(normal_view));
    vec3 coord = shadow_coord_from_world(world_position + n_world * 0.0025, false);
    if (coord.x <= 0.0 || coord.x >= 1.0 || coord.y <= 0.0 || coord.y >= 1.0 ||
        coord.z <= 0.0 || coord.z >= 1.0) return 1.0;
    ivec2 size_px = textureSize(shadow_depth, 0);
    vec2 texel = 1.0 / max(vec2(size_px), vec2(1.0));
    float ndotl = max(dot(normalize(normal_view), normalize(light_view)), 0.0);
    float bias = max(0.00030, 0.00090 * (1.0 - ndotl));
    float visible = 0.0;
    for (int y=-1;y<=1;++y) for (int x=-1;x<=1;++x) {
        float d = texture(shadow_depth, coord.xy + vec2(x,y)*texel).r;
        visible += (coord.z - bias <= d) ? 1.0 : 0.0;
    }
    return visible / 9.0;
}

float self_shadow_visibility(vec3 world_position, vec3 normal_view, vec3 light_view) {
    vec3 n_world = view_to_world_direction(normalize(normal_view));
    if (close_shadow_level() == 2u) {
        float depth_span = 2.0 * focused_shadow_depth_half_extent();
        float texel_m = 2.0 * focused_shadow_half_extent() /
                        float(textureSize(self_shadow_depth, 0).x);
        vec3 coord = shadow_coord_from_world(
            world_position + n_world * min(0.0010, texel_m), true);
        float ndotl = clamp(dot(normalize(normal_view), normalize(light_view)), 0.0, 1.0);
        float bias = (0.00020 + 0.00040 * (1.0 - ndotl)) / depth_span;
        return lsg_cinematic_shadow_visibility(self_shadow_depth, coord,
                                               bias, 0.0020 / depth_span);
    }
    vec3 coord = shadow_coord_from_world(world_position + n_world * 0.0010, true);
    if (coord.x <= 0.0 || coord.x >= 1.0 || coord.y <= 0.0 || coord.y >= 1.0 ||
        coord.z <= 0.0 || coord.z >= 1.0) return 1.0;
    ivec2 size_px = textureSize(self_shadow_depth, 0);
    vec2 texel = 1.0 / max(vec2(size_px), vec2(1.0));
    float ndotl = max(dot(normalize(normal_view), normalize(light_view)), 0.0);
    float bias = max(0.00018, 0.00065 * (1.0 - ndotl));
    float visible = 0.0;
    for (int y=-1;y<=1;++y) for (int x=-1;x<=1;++x) {
        float d = texture(self_shadow_depth, coord.xy + vec2(x,y)*texel).r;
        visible += (coord.z - bias <= d) ? 1.0 : 0.0;
    }
    return visible / 9.0;
}

float self_shadow_compare_margin(vec3 world_position, vec3 normal_view,
                                 vec3 light_view) {
    vec3 n_world = view_to_world_direction(normalize(normal_view));
    if (close_shadow_level() == 2u) {
        float depth_span = 2.0 * focused_shadow_depth_half_extent();
        float texel_m = 2.0 * focused_shadow_half_extent() /
                        float(textureSize(self_shadow_depth, 0).x);
        vec3 coord = shadow_coord_from_world(
            world_position + n_world * min(0.0010, texel_m), true);
        float ndotl = clamp(dot(normalize(normal_view), normalize(light_view)), 0.0, 1.0);
        float bias = (0.00020 + 0.00040 * (1.0 - ndotl)) / depth_span;
        return lsg_cinematic_shadow_margin(self_shadow_depth, coord,
                                           bias, 0.0020 / depth_span);
    }
    vec3 coord = shadow_coord_from_world(
        world_position + n_world * 0.0010, true);
    if (coord.x <= 0.0 || coord.x >= 1.0 ||
        coord.y <= 0.0 || coord.y >= 1.0 ||
        coord.z <= 0.0 || coord.z >= 1.0) {
        return 1.0;
    }
    float ndotl = max(dot(normalize(normal_view),
                          normalize(light_view)), 0.0);
    float bias = max(0.00018, 0.00065 * (1.0 - ndotl));
    float stored_depth = texture(self_shadow_depth, coord.xy).r;
    return stored_depth - (coord.z - bias);
}

vec3 sun_view_direction() {
    return normalize(rotate_x(rotate_y(sun_world_direction(), -pc.camera.x), -pc.camera.y));
}

vec3 procedural_environment(vec2 logical_ndc) {
    float tan_half_fov = tan(max(pc.render.y, 0.10) * 0.5);
    vec3 ray_view = normalize(vec3(logical_ndc.x * max(pc.render.x, 0.01) * tan_half_fov,
                                   logical_ndc.y * tan_half_fov,
                                   -1.0));
    vec3 ray_world = normalize(rotate_y(rotate_x(ray_view, pc.camera.y), pc.camera.x));
    vec3 sun_world = sun_world_direction();

    float mu = clamp(dot(ray_world, sun_world), -1.0, 1.0);
    float rayleigh_phase = 0.75 * (1.0 + mu * mu);
    const float g = 0.76;
    float mie_denom = max(1.0 + g * g - 2.0 * g * mu, 0.025);
    float mie_phase = (1.0 - g * g) / pow(mie_denom, 1.5);

    float elevation = clamp(ray_world.y, 0.0, 1.0);
    float horizon = pow(1.0 - elevation, 2.2);
    float sky_intensity = max(lighting.sun_tint_sky_intensity.w, 0.0);
    float direct_intensity = max(lighting.sun_direction_intensity.w, 0.0);
    vec3 zenith = lighting.sky_zenith_exposure.rgb;
    vec3 horizon_colour = lighting.sky_horizon_scene_lum.rgb;
    vec3 sun_tint = lighting.sun_tint_sky_intensity.rgb;

    float pol_attenuation = sky_polarization_attenuation(ray_world, sun_world);
    vec3 scattered = mix(zenith, horizon_colour, horizon);
    scattered += mix(zenith, horizon_colour, 0.35) * rayleigh_phase *
                 (0.18 + 0.52 * elevation);
    scattered += sun_tint * mie_phase * 0.012 * direct_intensity *
                 (0.35 + 0.65 * horizon);
    scattered *= sky_intensity * pol_attenuation;

    const float sun_inner = 0.999965;
    const float sun_outer = 0.99982;
    float sun_disk = smoothstep(sun_outer, sun_inner, mu);
    vec3 sky = scattered + sun_tint * 8.0 * sun_disk * direct_intensity;

    if (ray_world.y < 0.0) {
        float ndotl = max(sun_world.y, 0.0);
        vec3 ground = vec3(0.18) * horizon_colour *
                      (0.45 + 0.55 * sky_intensity) *
                      (0.30 + 0.70 * ndotl * direct_intensity);
        ground += zenith * 0.08 * sky_intensity;
        float horizon_blend = smoothstep(-0.035, 0.025, ray_world.y);
        sky = mix(ground, sky, horizon_blend);
    }

    float exposure = max(lighting.sky_zenith_exposure.w, 0.01);
    return aces_fitted(max(sky * bulk_filter_rgb() * exposure, vec3(0.0)));
}

vec3 sky_irradiance(vec3 n) {
    float up = clamp(n.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 horizon = lighting.sky_horizon_scene_lum.rgb;
    vec3 zenith = lighting.sky_zenith_exposure.rgb;
    return mix(horizon, zenith, pow(up, 0.65)) *
           max(lighting.sun_tint_sky_intensity.w, 0.0);
}

void main() {
    uint profile_index = pc.flags.x % 3u;
    bool detail_enabled = pc.flags.y != 0u;
    uint mode = diagnostic_mode();
    uint tooltip = (pc.flags.w >> 5u) & 15u;
    uint physiology = (pc.flags.w >> 9u) & 3u;
    uint eye_mode = (pc.flags.w >> 11u) & 3u;
    uint lighting_preset = (pc.flags.w >> 13u) & 3u;
    uint optical_filter = (pc.flags.w >> 15u) & 3u;
    uint surface_debug = surface_diagnostic_mode();

    if (ui_environment_pass()) {
        if (body_region == 200u) {
            out_colour = vec4(procedural_environment(surface_position_m.xy), 1.0);
            return;
        }
        if (body_region == 190u) {
            out_colour = vec4(0.055, 0.070, 0.095, 1.0);
            return;
        }
        if (body_region == 180u) {
            vec2 uv = surface_position_m.xy;
            vec3 colour = vec3(0.035, 0.045, 0.065);
            float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
            if (edge < 0.035) colour = vec3(0.20, 0.44, 0.72);
            if (tooltip_text_pixel(tooltip, uv)) colour = vec3(0.97, 0.985, 1.0);
            out_colour = vec4(colour, 1.0);
            return;
        }
        if (body_region >= 210u && body_region <= 212u) {
            if (!character_menu_open()) discard;
            uint option = body_region - 210u;
            vec2 uv = surface_position_m.xy;
            if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) discard;
            float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
            bool is_active = profile_index == option;
            vec3 panel = is_active ? vec3(0.12, 0.46, 0.78) : vec3(0.055, 0.075, 0.11);
            if (edge < 0.035) panel = min(panel + vec3(0.20), vec3(1.0));
            if (character_menu_text_pixel(option, uv)) panel = vec3(0.97, 0.985, 1.0);
            out_colour = vec4(panel, 1.0);
            return;
        }
        if (body_region < 100u || body_region > 111u) discard;

        uint button = body_region - 100u;
        vec2 uv = surface_position_m.xy;
        if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) discard;

        uint camera_preset = (pc.flags.w >> 3u) & 3u;
        bool selected = (button == 0u && profile_index == 0u) ||
                        (button == 1u && profile_index == 1u) ||
                        (button == 2u && detail_enabled) ||
                        (button == 3u && mode == 3u) ||
                        (button == 5u && (mode != 0u || surface_debug != 0u)) ||
                        (button == 7u && physiology != 0u) ||
                        (button == 8u && eye_mode != 0u) ||
                        (button == 9u) ||
                        (button == 10u) ||
                        (button == 11u && shadow_probe_active()) ||
                        (button == tooltip);

        float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        vec3 panel = selected ? vec3(0.12, 0.46, 0.78) : vec3(0.095, 0.12, 0.17);
        if (button == 3u && mode == 3u) panel = vec3(0.12, 0.60, 0.36);
        if (button == 4u) {
            panel = camera_preset == 0u ? vec3(0.16, 0.26, 0.48)
                  : camera_preset == 1u ? vec3(0.16, 0.42, 0.55)
                                        : vec3(0.50, 0.28, 0.12);
        }
        if (button == 5u) {
            panel = surface_debug == 1u ? vec3(0.12, 0.56, 0.30)
                  : surface_debug == 2u ? vec3(0.12, 0.38, 0.72)
                  : surface_debug == 3u ? vec3(0.68, 0.34, 0.12)
                  : surface_debug == 4u ? vec3(0.82, 0.22, 0.18)
                  : mode != 0u ? vec3(0.48, 0.26, 0.62)
                               : vec3(0.095, 0.12, 0.17);
        }
        if (button == 6u) panel = vec3(0.16, 0.18, 0.22);
        if (button == 7u) {
            panel = physiology == 0u ? vec3(0.12, 0.20, 0.22)
                  : physiology == 1u ? vec3(0.62, 0.18, 0.14)
                  : physiology == 2u ? vec3(0.12, 0.34, 0.62)
                                     : vec3(0.72, 0.32, 0.10);
        }

        if (button == 8u) {
            panel = eye_mode == 0u ? vec3(0.12, 0.24, 0.30)
                  : eye_mode == 1u ? vec3(0.52, 0.22, 0.62)
                  : eye_mode == 2u ? vec3(0.18, 0.52, 0.34)
                                   : vec3(0.14, 0.42, 0.72);
        }

        if (button == 9u) {
            panel = lighting_preset == 0u ? vec3(0.68, 0.42, 0.16)
                  : lighting_preset == 1u ? vec3(0.62, 0.66, 0.58)
                  : lighting_preset == 2u ? vec3(0.72, 0.28, 0.10)
                                          : vec3(0.08, 0.14, 0.34);
        }

        if (button == 10u) {
            panel = optical_filter == 0u ? vec3(0.34, 0.43, 0.50)
                  : optical_filter == 1u ? vec3(0.13, 0.25, 0.34)
                                         : vec3(0.22, 0.42, 0.58);
        }

        if (edge < 0.045) panel = min(panel + vec3(0.20), vec3(1.0));
        if (hud_glyph(button, uv)) panel = vec3(0.96, 0.98, 1.00);
        out_colour = vec4(panel, 1.0);
        return;
    }

    if (body_region == 201u) {
        vec3 n = normalize(view_normal);
        vec3 l = sun_view_direction();
        float ndotl = max(dot(n, l), 0.0);
        float visibility = coarse_shadow_visibility(surface_position_m, n, l);
        float direct = max(lighting.sun_direction_intensity.w, 0.0);
        float sky = max(lighting.sun_tint_sky_intensity.w, 0.0);
        float checker = mod(floor(surface_position_m.x) + floor(surface_position_m.z), 2.0);
        vec3 base = mix(vec3(0.155), vec3(0.175), checker);
        vec3 ambient = base * (0.24 + 0.24 * sky);
        vec3 sun = base * lighting.sun_tint_sky_intensity.rgb *
                   (0.82 * ndotl * direct * visibility);
        vec3 colour = (ambient + sun) * bulk_filter_rgb() *
                      max(lighting.sky_zenith_exposure.w, 0.01);
        out_colour = vec4(aces_fitted(max(colour, vec3(0.0))), 1.0);
        return;
    }

    // Region 255 is reserved for MakeHuman joint-marker geometry. It is only visible
    // in explicit Skeleton/Joint Debug mode and is not treated as skin.
    if (body_region == 255u) {
        if (mode != 3u) discard;
        vec3 n = normalize(view_normal);
        float ndotl = max(dot(n, sun_view_direction()), 0.0);
        vec3 joint_colour = vec3(0.04, 0.92, 0.52) * (0.55 + 0.45 * ndotl);
        out_colour = vec4(aces_fitted(joint_colour * 1.35), 1.0);
        return;
    }

    // Corrective-pass surface diagnostics. These are deliberately unlit
    // signal views so seams can be attributed to shadows, normals, or region tags.
    if (surface_debug != 0u) {
        vec3 n = normalize(view_normal);
        if (surface_debug == 1u) {
            vec3 l = sun_view_direction();
            float visibility = self_shadow_visibility(surface_position_m, n, l);
            out_colour = vec4(vec3(visibility), 1.0);
            return;
        }
        if (surface_debug == 4u) {
            vec3 l = sun_view_direction();
            float margin =
                self_shadow_compare_margin(surface_position_m, n, l);
            const float warning_band = 0.00075;
            vec3 compare_colour =
                margin > warning_band ? vec3(0.78)
              : margin < -warning_band ? vec3(0.055)
              : vec3(1.00, 0.24, 0.035);
            out_colour = vec4(compare_colour, 1.0);
            return;
        }
        if (surface_debug == 2u) {
            out_colour = vec4(n * 0.5 + 0.5, 1.0);
            return;
        }

        const vec3 palette[11] = vec3[11](
            vec3(0.92,0.22,0.20), vec3(0.95,0.58,0.16), vec3(0.86,0.80,0.18),
            vec3(0.30,0.74,0.28), vec3(0.14,0.72,0.62), vec3(0.18,0.52,0.88),
            vec3(0.38,0.34,0.88), vec3(0.70,0.30,0.86), vec3(0.86,0.28,0.58),
            vec3(0.62,0.52,0.40), vec3(0.62,0.68,0.76));
        uint region = min(body_region, 10u);
        out_colour = vec4(palette[region], 1.0);
        return;
    }

    // RAW diagnostic modes deliberately bypass the entire LSG surface function.
    if (mode == 1u || mode == 2u) {
        vec3 n = normalize(view_normal);
        vec3 l = sun_view_direction();
        float ndotl = max(dot(n, l), 0.0);
        vec3 neutral = vec3(0.48, 0.43, 0.39);
        vec3 colour = neutral * (0.24 + 0.76 * ndotl) + neutral * sky_irradiance(n) * 0.22;
        out_colour = vec4(aces_fitted(colour * bulk_filter_rgb() * 1.15 *
                                      max(lighting.sky_zenith_exposure.w, 0.01)), 1.0);
        return;
    }

    uint seed = floatBitsToUint(pc.render.w);
    float footprint_m = max(length(dFdx(surface_position_m)), length(dFdy(surface_position_m)));
    int band = detail_band(footprint_m * 1000.0);

    float melanin = clamp(pc.skin0.x, 0.0, 1.0);
    float haemoglobin = clamp(pc.skin0.y, 0.0, 1.0);
    float oiliness = clamp(pc.skin0.z, 0.0, 1.0);
    float hydration = clamp(pc.skin0.w, 0.0, 1.0);

    // Render-control physiology approximation from the v0 contract.
    // 0 normal, 1 exercise, 2 cold, 3 hot.
    float perfusion = physiology == 1u ? 0.84
                    : physiology == 2u ? 0.22
                    : physiology == 3u ? 0.66 : 0.45;
    float sweat = physiology == 1u ? 0.72
                : physiology == 2u ? 0.03
                : physiology == 3u ? 0.88 : 0.08;
    haemoglobin = clamp(haemoglobin + (perfusion - 0.45) * 0.36, 0.0, 1.0);

    vec3 light_skin = vec3(0.66, 0.39, 0.29);
    vec3 dark_skin = vec3(0.12, 0.050, 0.030);
    vec3 base_colour = mix(light_skin, dark_skin, pow(melanin, 0.82) * 0.90);
    base_colour += vec3(0.08, 0.010, 0.005) * (haemoglobin - 0.45);
    if (physiology == 2u) base_colour += vec3(-0.010, 0.006, 0.028);
    if (physiology == 3u) base_colour += vec3(0.030, 0.004, -0.008);

    float meso = 0.0;
    float pore_influence = 0.0;
    float height_field = 0.0;
    float roughness = clamp(0.62 + (pc.micro0.x - 0.5) * 0.26 - oiliness * 0.16 - hydration * 0.05
                            - sweat * 0.10,
                            0.24, 0.86);

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
        float density = clamp(
            pc.micro0.y * anatomical_pore_density_scale(surface_position_m),
            0.05, 0.95);
        float depth_m = mix(0.000010, 0.000050, clamp(pc.micro0.w, 0.0, 1.0));
        vec2 pore = pore_field(surface_position_m, seed, cell_m, density, depth_m);
        height_field += pore.x;
        pore_influence = pore.y;
        roughness = clamp(roughness + pore_influence * 0.10, 0.25, 0.95);
    }

    if (detail_enabled && band >= 3) {
        float subpixel = value_noise(surface_position_m / 0.00012, seed ^ 0xC2B2AE35u) - 0.5;
        float filter_weight = clamp((0.00010 - footprint_m) / 0.00010, 0.0, 1.0);
        height_field += subpixel * 0.000004 * filter_weight;
        roughness = clamp(roughness + subpixel * 0.035 * filter_weight, 0.24, 0.95);
    }

    vec3 n = normalize(view_normal);
    if (detail_enabled && band >= 1) n = perturb_normal(n, view_position_m, height_field, 2.2);
    vec3 v = normalize(-view_position_m);

    vec3 l = sun_view_direction();
    vec3 h = normalize(v + l);
    float ndotl = max(dot(n, l), 0.0);
    float ndotv = max(dot(n, v), 0.001);
    float ndoth = max(dot(n, h), 0.0);
    float hdotv = max(dot(h, v), 0.0);

    const float skin_f0_scalar = 0.0277778;
    vec3 f0 = vec3(skin_f0_scalar);
    vec3 fresnel = fresnel_schlick(hdotv, f0);
    float d = distribution_ggx(ndoth, roughness);
    float g = geometry_schlick(ndotv, roughness) * geometry_schlick(max(ndotl, 0.001), roughness);
    vec3 specular = fresnel * (d * g / max(4.0 * ndotv * max(ndotl, 0.001), 0.001));

    // Direct-light energy must vanish when the sun is behind the surface.
    // Keep the intentionally bounded SSS/back-scatter approximation separate.
    vec3 diffuse = base_colour * ndotl * (1.0 - fresnel) / 3.14159265;
    // Bounded v0 skin-transmission approximation. The previous term peaked when
    // NdotL approached zero and produced an inverted bright band at the terminator.
    // This replacement keeps a restrained contribution near the lit terminator and
    // fades it before the back-facing side.
    float signed_ndotl = dot(n, l);
    float sss_wrap = smoothstep(-0.04, 0.28, signed_ndotl);
    float sss_terminator = (1.0 - ndotl) * sss_wrap;
    vec3 subsurface_approx =
        base_colour * vec3(1.05, 0.45, 0.32) *
        sss_terminator * 0.016;

    vec3 sky = sky_irradiance(n);
    vec3 ambient = base_colour * sky * 0.28;
    vec3 sun_radiance = lighting.sun_tint_sky_intensity.rgb *
                        (3.1 * max(lighting.sun_direction_intensity.w, 0.0));
    float direct_visibility =
        // Micro-normal detail changes the BRDF, not shadow-map geometry/bias.
        // Match Shadow Visibility/Compare, which use the base smooth normal.
        self_shadow_visibility(surface_position_m, normalize(view_normal), l);
    float direct_specular_scale =
        (1.2 + oiliness * 0.75 + sweat * 0.35) * ndotl;
    vec3 direct_colour =
        diffuse * sun_radiance +
        specular * sun_radiance * direct_specular_scale;
    vec3 colour = ambient + direct_colour * direct_visibility +
                  subsurface_approx * (0.55 + 0.45 * sky);

    colour *= bulk_filter_rgb() * 1.05 *
              max(lighting.sky_zenith_exposure.w, 0.01);
    out_colour = vec4(aces_fitted(max(colour, vec3(0.0))), 1.0);
}
