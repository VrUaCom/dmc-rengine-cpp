#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uint in_region;

layout(location = 0) out vec3 surface_position_m;
layout(location = 1) out vec3 surface_normal;
layout(location = 2) flat out uint body_region;

layout(push_constant) uniform LsgPush {
    vec4 center_scale;
    vec4 render_params; // aspect, metres-per-source-unit, time, reserved
    uvec4 flags;        // character, detail, reserved, reserved
} pc;

void main() {
    vec3 local = in_position - pc.center_scale.xyz;
    uint character_index = pc.flags.x & 1u;

    float width_scale = 1.0;
    float depth_scale = 1.0;
    float height_scale = character_index == 0u ? 1.012 : 0.988;
    if (character_index == 0u) {
        if (in_region == 2u || in_region == 3u || in_region == 5u) width_scale = 1.055;
        if (in_region == 4u) width_scale = 0.990;
        if (in_region == 2u || in_region == 3u) depth_scale = 1.035;
    } else {
        if (in_region == 2u || in_region == 3u || in_region == 5u) width_scale = 0.965;
        if (in_region == 8u || in_region == 9u) width_scale = 1.035;
        if (in_region == 0u) width_scale = 1.012;
    }

    vec3 shaped = local;
    shaped.x *= width_scale;
    shaped.y *= height_scale;
    shaped.z *= depth_scale;

    float scale = pc.center_scale.w;
    float aspect = max(pc.render_params.x, 0.01);
    gl_Position = vec4(shaped.x * scale / aspect,
                       shaped.y * scale,
                       0.50 - shaped.z * scale * 0.12,
                       1.0);

    surface_position_m = shaped * pc.render_params.y;
    surface_normal = normalize(vec3(in_normal.x / width_scale,
                                    in_normal.y / height_scale,
                                    in_normal.z / depth_scale));
    body_region = in_region;
}
