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

vec3 view_to_world_direction(vec3 view_direction) {
    return normalize(rotate_y(rotate_x(view_direction, pc.camera.y), pc.camera.x));
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

float reflected_sky_polarization_attenuation(vec3 ray_world, vec3 sun_world) {
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

float eye_local_hash(uint side, ivec2 cell, uint salt) {
    uint h = pcg_hash(pc.flags.w ^ (side == 0u ? 0xA511E9B3u : 0x63D83595u) ^ salt);
    h = pcg_hash(h ^ uint(cell.x) * 0x9e3779b9u);
    h = pcg_hash(h ^ uint(cell.y) * 0x85ebca6bu);
    return hash01(h);
}

float eye_local_value_noise(uint side, vec2 p, uint salt) {
    ivec2 cell = ivec2(floor(p));
    vec2 local = fract(p);
    vec2 w = local * local * (3.0 - 2.0 * local);
    float a = eye_local_hash(side, cell, salt);
    float b = eye_local_hash(side, cell + ivec2(1, 0), salt);
    float c = eye_local_hash(side, cell + ivec2(0, 1), salt);
    float d = eye_local_hash(side, cell + ivec2(1, 1), salt);
    return mix(mix(a, b, w.x), mix(c, d, w.x), w.y);
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

    // Eye-local, deterministic low-frequency variation. The luminance swing stays
    // deliberately restrained to avoid a blotchy or dirty sclera.
    float low = eye_local_value_noise(side, local_uv * 3.25 + vec2(7.0, 11.0), 0x51C3A11u);
    float low_signed = (low - 0.5) * 2.0;
    base *= 1.0 + low_signed * 0.012;
    base += vec3(0.0020, -0.0005, -0.0015) * low_signed;

    // Sparse directional vascular hints: a softly warped ridge field, gated by
    // peripheral distance and a low-frequency deterministic mask.
    float radius = length(local_uv);
    float edge_weight = smoothstep(0.34, 0.96, radius);
    float vessel_mask = eye_local_value_noise(
        side, local_uv * vec2(2.4, 3.2) + vec2(19.0, 5.0), 0xB10D5EEDu);
    float phase = eye_local_hash(side, ivec2(3, 7), 0x7E5511u) * 6.28318530718;
    float directional =
        local_uv.y * 5.2 + local_uv.x * 1.35 +
        0.32 * sin(local_uv.y * 7.0 + phase);
    float ridge = 1.0 - abs(fract(directional) * 2.0 - 1.0);
    ridge = pow(clamp(ridge, 0.0, 1.0), 18.0);
    float sparse = smoothstep(0.72, 0.92, vessel_mask);
    float vessel = ridge * sparse * edge_weight * vascularity;
    base += vec3(0.050, -0.009, -0.013) * vessel * 0.22;

    return clamp(base, vec3(0.0), vec3(1.0));
}

vec3 procedural_inner_eye(uint component, vec2 uv) {
    uint side = component == 1u ? 0u : 1u;
    vec2 local = inner_eye_local_uv(component, uv);
    float radius = length(local);
    float angle = atan(local.y, local.x);
    float angle01 = angle * (0.5 / 3.141592653589793) + 0.5;

    const float iris_radius = 0.295;
    float pupil_radius = clamp(pc.eye0.a, 0.045, 0.135);
    float iris_mask = 1.0 - smoothstep(iris_radius - 0.017, iris_radius + 0.010, radius);
    float pupil_mask = 1.0 - smoothstep(pupil_radius - 0.008, pupil_radius + 0.006, radius);

    vec3 sclera = sclera_colour(local, side);

    float radial01 = clamp(radius / iris_radius, 0.0, 1.0);

    // Stable low-frequency angular phase warp breaks the perfect radial regularity
    // without introducing screen-space noise.
    float warp_coarse = iris_variation(side, angle01 * 24.0 + 5.0, 29) - 0.5;
    float warp_radial = iris_variation(
        side, angle01 * 40.0 + radial01 * 7.0 + 17.0,
        int(floor(radial01 * 6.0)) + 37) - 0.5;
    float warped_angle = angle01 + warp_coarse * 0.020 + warp_radial * radial01 * 0.008;

    int coarse_band = int(floor(radial01 * 6.0));
    int medium_band = int(floor(radial01 * 12.0));
    int fine_band = int(floor(radial01 * 18.0));
    float coarse = iris_variation(side, warped_angle * 48.0 + 3.0, coarse_band);
    float medium = iris_variation(side, warped_angle * 144.0 + 11.0, medium_band + 9);
    float fine = iris_variation(side, warped_angle * 288.0 + 23.0, fine_band + 21);

    // Attenuate the finest field as the fragment footprint grows.
    float footprint = max(fwidth(local.x), fwidth(local.y));
    float fine_visibility = 1.0 - smoothstep(0.010, 0.035, footprint);
    float fibre_mix = clamp(
        0.25 * coarse + 0.45 * medium + 0.30 * mix(0.5, fine, fine_visibility),
        0.0, 1.0);

    vec3 iris = mix(pc.eye0.rgb, pc.eye1.rgb, fibre_mix);
    float radial_darkening = mix(1.08, 0.72, smoothstep(0.0, 1.0, radial01));
    iris *= radial_darkening;

    float collarette_noise =
        iris_variation(side, warped_angle * 32.0 + 7.0, 41) - 0.5;
    float collarette_radius = 0.47 + collarette_noise * 0.070;
    float collarette = exp(-pow((radial01 - collarette_radius) / 0.10, 2.0));
    iris += pc.eye1.rgb * collarette * (0.065 + 0.090 * medium);

    float limbal_noise =
        iris_variation(side, warped_angle * 36.0 + 31.0, 53) - 0.5;
    float limbal_start = 0.785 + limbal_noise * 0.055;
    float limbal = smoothstep(limbal_start, 1.0, radial01);
    float limbal_strength = 0.57 + limbal_noise * 0.10;
    iris *= mix(1.0, clamp(limbal_strength, 0.48, 0.66), limbal);

    vec3 pupil = vec3(0.008, 0.010, 0.012);
    vec3 colour = mix(sclera, iris, iris_mask);
    colour = mix(colour, pupil, pupil_mask);

    vec3 n = normalize(view_normal);
    vec3 light_dir = sun_view_direction();
    vec3 v = normalize(-view_position);
    vec3 h = normalize(light_dir + v);
    float ndotl = max(dot(n, light_dir), 0.0);
    float direct = max(lighting.sun_direction_intensity.w, 0.0);
    float sky = max(lighting.sun_tint_sky_intensity.w, 0.0);
    vec3 illumination = vec3(0.42 * sky) +
                        lighting.sun_tint_sky_intensity.rgb * (0.58 * ndotl * direct);
    colour *= illumination * max(lighting.sky_zenith_exposure.w, 0.01);

    // Pass 4D wet-eye approximation. The inner-eye pipeline is intentionally opaque,
    // so this is a restrained radiance/specular contribution rather than fake alpha.
    // It is strongest near the exposed peripheral sclera and biased toward the lower edge.
    float wet_edge = smoothstep(0.76, 0.98, radius);
    float lower_bias = smoothstep(0.05, 0.82, -local.y);
    float sclera_gate = 1.0 - iris_mask * 0.92;
    float wet_weight = wet_edge * mix(0.28, 1.0, lower_bias) * sclera_gate;
    float wet_sun = pow(max(dot(n, h), 0.0), 180.0) * direct;
    float wet_grazing = pow(1.0 - max(dot(n, v), 0.0), 4.0) * sky;
    float wet_intensity = wet_weight *
                          (0.010 * sky + 0.095 * wet_sun + 0.024 * wet_grazing);
    colour += vec3(0.985, 0.995, 1.0) * wet_intensity;

    return colour * bulk_filter_rgb();
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
    vec3 reflected_world = view_to_world_direction(reflected);
    vec3 sun_world = normalize(lighting.sun_direction_intensity.xyz);
    float pol_attenuation =
        reflected_sky_polarization_attenuation(reflected_world, sun_world);

    vec3 sky = mix(lighting.sky_horizon_scene_lum.rgb,
                   lighting.sky_zenith_exposure.rgb,
                   sky_factor) * max(lighting.sun_tint_sky_intensity.w, 0.0) *
                   pol_attenuation;

    // Keep the wet shell nearly colourless. The environment still drives intensity,
    // but only a restrained fraction of its chroma is allowed into the cornea.
    float sky_luminance = dot(sky, vec3(0.2126, 0.7152, 0.0722));
    vec3 neutral_sky = vec3(sky_luminance);
    vec3 restrained_sky = mix(neutral_sky, sky, 0.16);

    // Polarized Approx intentionally attenuates the reflected-sky/glare proxy only.
    // The sharp direct-sun highlight remains bulk-transmission-only in v0.
    float environment_reflectance = 0.018 + fresnel * 0.52;
    vec3 colour = restrained_sky * environment_reflectance +
                  lighting.sun_tint_sky_intensity.rgb * sun_spec * 1.85;
    colour *= bulk_filter_rgb() * max(lighting.sky_zenith_exposure.w, 0.01);

    // Bounded transparent shell: enough Fresnel to read as a wet cornea without
    // recreating the previous blue/plastic sphere.
    float alpha = clamp(0.025 + fresnel * 0.255 + sun_spec * 0.18, 0.025, 0.38);
    return vec4(colour, alpha);
}

void main() {
    uint eye_mode = (pc.flags.z >> 8u) & 3u;
    uint eye_pass = (pc.flags.z >> 10u) & 3u;
    bool inner = component_id == 1u || component_id == 3u;
    bool outer = component_id == 0u || component_id == 2u;

    if (eye_pass == 0u) {
        if (!inner) discard;
    } else if (eye_pass == 1u) {
        if (!outer) discard;
    } else {
        discard;
    }

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
        if (length(local) > 0.320) discard;
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
