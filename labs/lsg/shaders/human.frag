#version 450

layout(location = 0) in vec2 surface_position_m;
layout(location = 1) in vec3 bary_colour;
layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform LsgPush {
    uint character_index;
    uint detail_enabled;
    float time_seconds;
    float reserved;
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

float lsg_hash01(uint h) {
    return float(h >> 8u) * (1.0 / 16777216.0);
}

int lsg_detail_band(float mm_per_pixel) {
    if (mm_per_pixel < 0.1) return 3;
    if (mm_per_pixel < 1.0) return 2;
    if (mm_per_pixel < 3.0) return 1;
    return 0;
}

void main() {
    vec3 base_skin = pc.character_index == 0u
        ? vec3(0.56, 0.31, 0.22)
        : vec3(0.49, 0.29, 0.24);
    uvec2 seed = pc.character_index == 0u
        ? uvec2(0xFFEE1234u, 0x000000C0u)
        : uvec2(0xADBEEF42u, 0x000000DEu);

    float footprint_m = max(length(dFdx(surface_position_m)), length(dFdy(surface_position_m)));
    float mm_per_pixel = footprint_m * 1000.0;
    int band = lsg_detail_band(mm_per_pixel);

    const float pore_cell_m = 0.00034;
    ivec3 cell = ivec3(floor(vec3(surface_position_m, 0.0) / pore_cell_m));
    float pore_random = lsg_hash01(lsg_hash5(seed, 0u, cell));
    float meso = lsg_hash01(lsg_hash5(seed ^ uvec2(0xA511E9B3u, 0u), 0u, cell / 16)) - 0.5;

    float surface_gain = 1.0;
    if (pc.detail_enabled != 0u && band >= 1) {
        surface_gain += meso * 0.09;
    }
    if (pc.detail_enabled != 0u && band >= 2 && pore_random < 0.58) {
        surface_gain -= (0.58 - pore_random) * 0.13;
    }

    // Subtle vertex tint makes the rasterized triangle unmistakable while the
    // stable rest-space field demonstrates the LSG deterministic GPU contract.
    vec3 colour = base_skin * surface_gain + bary_colour * 0.035;
    out_colour = vec4(max(colour, vec3(0.0)), 1.0);
}
