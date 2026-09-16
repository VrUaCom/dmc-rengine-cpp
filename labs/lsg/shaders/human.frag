#version 450

layout(location = 0) in vec3 surface_position_m;
layout(location = 1) in vec3 surface_normal;
layout(location = 2) flat in uint body_region;
layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform LsgPush {
    vec4 center_scale;
    vec4 render_params;
    uvec4 flags;
} pc;

uint lsg_pcg_hash(uint input_value) {
    uint state = input_value * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

uint lsg_hash5(uvec2 seed, uint region, ivec3 cell) {
    uint h = lsg_pcg_hash(seed.x ^ seed.y);
    h = lsg_pcg_hash(h ^ region * 0x9e3779b9u);
    h = lsg_pcg_hash(h ^ uint(cell.x));
    h = lsg_pcg_hash(h ^ uint(cell.y));
    h = lsg_pcg_hash(h ^ uint(cell.z));
    return h;
}

float lsg_hash01(uint h) { return float(h >> 8u) * (1.0 / 16777216.0); }
int lsg_detail_band(float mm_per_pixel) { if (mm_per_pixel < 0.1) return 3; if (mm_per_pixel < 1.0) return 2; if (mm_per_pixel < 3.0) return 1; return 0; }

float region_pore_density(uint region) {
    if (region == 0u) return 0.68;
    if (region == 7u) return 0.58;
    if (region == 1u || region == 2u) return 0.52;
    return 0.43;
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

void main() {
    uint character_index = pc.flags.x & 1u;
    bool detail_enabled = pc.flags.y != 0u;

    if (pc.flags.w != 0u) {
        uint button = body_region >= 100u ? body_region - 100u : 0u;
        vec2 uv = surface_position_m.xy;
        bool active = (button == 0u && character_index == 0u) ||
                      (button == 1u && character_index == 1u) ||
                      (button == 2u && detail_enabled);
        float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        vec3 panel = active ? vec3(0.16, 0.56, 0.92) : vec3(0.12, 0.16, 0.22);
        if (edge < 0.055) panel = active ? vec3(0.45, 0.78, 1.00) : vec3(0.38, 0.43, 0.50);
        if (hud_glyph(button, uv)) panel = vec3(0.96, 0.98, 1.00);
        out_colour = vec4(panel, 1.0);
        return;
    }

    vec3 base_skin = character_index == 0u ? vec3(0.56, 0.31, 0.22) : vec3(0.49, 0.29, 0.24);
    uvec2 seed = character_index == 0u ? uvec2(0xFFEE1234u, 0x000000C0u) : uvec2(0xADBEEF42u, 0x000000DEu);

    float footprint_m = max(length(dFdx(surface_position_m)), length(dFdy(surface_position_m)));
    float mm_per_pixel = footprint_m * 1000.0;
    int band = lsg_detail_band(mm_per_pixel);

    const float pore_cell_m = 0.00034;
    ivec3 pore_cell = ivec3(floor(surface_position_m / pore_cell_m));
    ivec3 meso_cell = ivec3(floor(surface_position_m / 0.0065));
    float pore_random = lsg_hash01(lsg_hash5(seed, body_region, pore_cell));
    float meso = lsg_hash01(lsg_hash5(seed ^ uvec2(0xA511E9B3u, 0u), body_region, meso_cell)) - 0.5;

    float surface_gain = 1.0;
    if (detail_enabled && band >= 1) surface_gain += meso * 0.075;
    float density = region_pore_density(body_region);
    if (detail_enabled && band >= 2 && pore_random < density) surface_gain -= (density - pore_random) * 0.10;

    vec3 n = normalize(surface_normal);
    vec3 sun_direction = normalize(vec3(0.34, 0.72, 0.60));
    float diffuse = 0.32 + 0.68 * max(dot(n, sun_direction), 0.0);
    float facing = 0.10 * pow(max(1.0 - abs(n.z), 0.0), 2.0);
    vec3 colour = base_skin * surface_gain * diffuse + vec3(facing);
    out_colour = vec4(max(colour, vec3(0.0)), 1.0);
}
