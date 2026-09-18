#version 450

layout(location = 0) in vec3 view_normal;
layout(location = 1) in vec2 eye_uv;
layout(location = 2) flat in uint component_id;
layout(location = 3) in vec3 view_position;

layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform EyePush {
    vec4 center_units;
    vec4 camera;
    vec4 geometry0;
    vec4 geometry1;
    vec4 render;
    vec4 eye0;         // primary iris rgb, pupil bias
    vec4 eye1;         // secondary iris rgb, sclera tint
    uvec4 flags;       // rotation, profile, vascularity byte, eye seed low
} pc;

layout(set = 0, binding = 0, std140) uniform FrameLighting {
    vec4 sun_direction_intensity;
    vec4 sun_tint_sky_intensity;
    vec4 sky_zenith_exposure;
    vec4 sky_horizon_scene_lum;
    vec4 filter_tint_transmission;
    vec4 eye_filter_misc;
    uvec4 modes;
} lighting;

vec3 rotate_y(vec3 v, float a) {
    float cs = cos(a), sn = sin(a);
    return vec3(cs * v.x + sn * v.z, v.y, -sn * v.x + cs * v.z);
}

vec3 rotate_x(vec3 v, float a) {
    float cs = cos(a), sn = sin(a);
    return vec3(v.x, cs * v.y - sn * v.z, sn * v.y + cs * v.z);
}

vec3 sun_view_direction() {
    vec3 sun_world = normalize(lighting.sun_direction_intensity.xyz);
    return normalize(rotate_x(rotate_y(sun_world, -pc.camera.x), -pc.camera.y));
}

uint pcg_hash(uint input_value) {
    uint state = input_value * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float hash01(uint h) {
    return float(h >> 8u) * (1.0 / 16777216.0);
}

uint iris_hash(uint side, int sector, int radial_band) {
    uint side_key = side == 0u ? 0xA511E9B3u : 0x63D83595u;
    uint h = pcg_hash(pc.flags.w ^ side_key);
    h = pcg_hash(h ^ 0x455945u * 0x9e3779b9u);
    h = pcg_hash(h ^ uint(sector));
    h = pcg_hash(h ^ uint(radial_band));
    h = pcg_hash(h ^ side);
    return h;
}

float iris_variation(uint side, float sector_f, int radial_band) {
    int sector0 = int(floor(sector_f));
    int sector1 = sector0 + 1;
    float t = smoothstep(0.0, 1.0, fract(sector_f));
    float a = hash01(iris_hash(side, sector0, radial_band));
    float b = hash01(iris_hash(side, sector1, radial_band));
    return mix(a, b, t);
}

vec2 inner_eye_local_uv(uint component, vec2 uv) {
    if (component == 1u) {
        const vec2 center = vec2(0.29870, 0.29435);
        const vec2 half_extent = vec2(0.26250, 0.26645);
        return (uv - center) / half_extent;
    }
    const vec2 center = vec2(0.69960, 0.69515);
    const vec2 half_extent = vec2(0.27700, 0.28115);
    return (uv - center) / half_extent;
}

vec3 sclera_colour(vec2 local_uv, uint side) {
    float tint = clamp(pc.eye1.a, 0.0, 1.0);
    float vascularity = float(pc.flags.z & 255u) * (1.0 / 255.0);

    vec3 warm = vec3(0.935, 0.905, 0.875);
    vec3 neutral = vec3(0.965, 0.955, 0.935);
    vec3 base = mix(warm, neutral, tint);

    ivec2 cell = ivec2(floor((local_uv + vec2(1.25)) * 18.0));
    uint h = pcg_hash(pc.flags.w ^ (side == 0u ? 0x4A39B70Du : 0xC13FA9A9u));
    h = pcg_hash(h ^ uint(cell.x));
    h = pcg_hash(h ^ uint(cell.y) * 0x9e3779b9u);
    float vessel = hash01(h);
    float sparse = smoothstep(0.90, 0.985, vessel);
    float edge_weight = smoothstep(0.28, 0.95, length(local_uv));
    base += vec3(0.075, -0.012, -0.018) * sparse * vascularity * edge_weight * 0.55;
    return base;
}

vec3 procedural_inner_eye(uint component, vec2 uv) {
    uint side = component == 1u ? 0u : 1u;
    vec2 local = inner_eye_local_uv(component, uv);
    float radius = length(local);
    float angle = atan(local.y, local.x);
    float angle01 = angle * (0.5 / 3.141592653589793) + 0.5;

    const float iris_radius = 0.345;
    float pupil_radius = clamp(pc.eye0.a, 0.070, 0.155);
    float iris_mask = 1.0 - smoothstep(iris_radius - 0.020, iris_radius + 0.012, radius);
    float pupil_mask = 1.0 - smoothstep(pupil_radius - 0.010, pupil_radius + 0.008, radius);

    vec3 sclera = sclera_colour(local, side);

    float radial01 = clamp(radius / iris_radius, 0.0, 1.0);
    float sectors = angle01 * 192.0;
    int radial_band = int(floor(radial01 * 14.0));
    float fibre = iris_variation(side, sectors, radial_band);
    float fibre2 = iris_variation(side, sectors * 0.5 + 13.0, radial_band + 7);
    float fibre_mix = clamp(0.18 + fibre * 0.58 + fibre2 * 0.24, 0.0, 1.0);

    vec3 iris = mix(pc.eye0.rgb, pc.eye1.rgb, fibre_mix);
    float radial_darkening = mix(1.08, 0.72, smoothstep(0.0, 1.0, radial01));
    iris *= radial_darkening;

    float collarette = exp(-pow((radial01 - 0.48) / 0.12, 2.0));
    iris += pc.eye1.rgb * collarette * (0.08 + 0.08 * fibre);

    float limbal = smoothstep(0.76, 1.0, radial01);
    iris *= mix(1.0, 0.38, limbal);

    vec3 pupil = vec3(0.008, 0.010, 0.012);
    vec3 colour = mix(sclera, iris, iris_mask);
    colour = mix(colour, pupil, pupil_mask);

    vec3 n = normalize(view_normal);
    vec3 light_dir = sun_view_direction();
    float ndotl = max(dot(n, light_dir), 0.0);
    float direct = max(lighting.sun_direction_intensity.w, 0.0);
    float sky = max(lighting.sun_tint_sky_intensity.w, 0.0);
    vec3 illumination = vec3(0.42 * sky) +
                        lighting.sun_tint_sky_intensity.rgb * (0.58 * ndotl * direct);
    colour *= illumination * max(lighting.sky_zenith_exposure.w, 0.01);
    return colour;
}

vec3 diagnostic_component_colour(uint id) {
    if (id == 0u) return vec3(0.95, 0.18, 0.16);
    if (id == 1u) return vec3(0.12, 0.78, 0.30);
    if (id == 2u) return vec3(0.18, 0.38, 0.96);
    return vec3(0.96, 0.72, 0.10);
}

vec4 cornea_response() {
    // Approximate dielectric cornea/wet-shell response, not full physical refraction.
    const float ior = 1.376;
    const float f0_scalar = ((ior - 1.0) / (ior + 1.0)) * ((ior - 1.0) / (ior + 1.0));
    vec3 n = normalize(view_normal);
    vec3 v = normalize(-view_position);
    float nv = max(dot(n, v), 0.0);
    float fresnel = f0_scalar + (1.0 - f0_scalar) * pow(1.0 - nv, 5.0);

    vec3 sun_dir = sun_view_direction();
    vec3 h = normalize(sun_dir + v);
    float direct = max(lighting.sun_direction_intensity.w, 0.0);
    float sun_spec = pow(max(dot(n, h), 0.0), 220.0) * direct;

    vec3 reflected = reflect(-v, n);
    float sky_factor = clamp(reflected.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 sky = mix(lighting.sky_horizon_scene_lum.rgb,
                   lighting.sky_zenith_exposure.rgb,
                   sky_factor) * max(lighting.sun_tint_sky_intensity.w, 0.0);

    vec3 colour = sky * (0.08 + fresnel * 1.55) +
                  lighting.sun_tint_sky_intensity.rgb * sun_spec * 2.8;
    colour *= max(lighting.sky_zenith_exposure.w, 0.01);
    float alpha = clamp(0.055 + fresnel * 0.62 + sun_spec * 0.52, 0.045, 0.72);
    return vec4(colour, alpha);
}

void main() {
    uint eye_mode = (pc.flags.z >> 8u) & 3u;
    bool inner = component_id == 1u || component_id == 3u;
    bool outer = component_id == 0u || component_id == 2u;

    if (eye_mode == 1u) {
        vec3 n = normalize(view_normal);
        vec3 light_dir = sun_view_direction();
        float ndotl = max(dot(n, light_dir), 0.0);
        float direct = max(lighting.sun_direction_intensity.w, 0.0);
        float sky = max(lighting.sun_tint_sky_intensity.w, 0.0);
        vec3 c = diagnostic_component_colour(component_id) *
                 (0.35 + 0.35 * sky + 0.30 * ndotl * direct);
        out_colour = vec4(c, 0.78);
        return;
    }

    if (eye_mode == 2u) {
        if (!inner) discard;
        vec2 local = inner_eye_local_uv(component_id, eye_uv);
        if (length(local) > 0.365) discard;
        out_colour = vec4(procedural_inner_eye(component_id, eye_uv), 1.0);
        return;
    }

    if (eye_mode == 3u) {
        if (!outer) discard;
        out_colour = cornea_response();
        return;
    }

    if (inner) {
        out_colour = vec4(procedural_inner_eye(component_id, eye_uv), 1.0);
        return;
    }

    if (outer) {
        out_colour = cornea_response();
        return;
    }

    discard;
}
